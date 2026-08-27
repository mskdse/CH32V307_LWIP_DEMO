/**
 * @file
 * Ethernet netif for lwIP on CH32V307 (internal 10M PHY, raw DMA driver).
 *
 * Event-driven RX/TX with OS synchronisation (ST lwIP_RTOS style):
 *  - the ETH DMA ISR signals eth_rx_semaphore on every received frame and
 *    eth_tx_semaphore on every TX completion (see ethernetif_rx/tx_signal);
 *  - ethernetif_input() blocks on eth_rx_semaphore, then drains all pending
 *    frames and pushes them into the tcpip_thread mailbox via netif->input;
 *  - low_level_output() waits on eth_tx_semaphore when the DMA still owns
 *    every TX descriptor, instead of dropping the frame.
 * Cross-thread protection of the lwIP core comes from lwIP's own core lock
 * (LWIP_TCPIP_CORE_LOCKING), implemented as a recursive FreeRTOS mutex in
 * the sys_arch port.
 */

#include "lwip/opt.h"

#include "lwip/def.h"
#include "lwip/debug.h"
#include "lwip/err.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/etharp.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "eth_driver.h"
#include "ethernetif.h"

#define IFNAME0 'e'
#define IFNAME1 'n'

/* Semaphores shared with the ETH DMA ISR (see ethernetif.h). */
static SemaphoreHandle_t eth_rx_semaphore;
static SemaphoreHandle_t eth_tx_semaphore;

void ethernetif_sync_init(void)
{
    eth_rx_semaphore = xSemaphoreCreateBinary();
    eth_tx_semaphore = xSemaphoreCreateBinary();
    configASSERT(eth_rx_semaphore != NULL);
    configASSERT(eth_tx_semaphore != NULL);
}

/* Called from the ETH ISR: wake the netif task that a frame is pending. */
void ethernetif_rx_signal(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(eth_rx_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Called from the ETH ISR: a TX descriptor has been freed by the DMA. */
void ethernetif_tx_signal(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(eth_tx_semaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*********************************************************************
 * @fn      low_level_init
 *
 * @brief   Hardware-independent part of netif setup.
 *          netif->hwaddr is pre-filled by LwIP_Init() before netif_add().
 */
static void low_level_init(struct netif *netif)
{
    netif->hwaddr_len = ETHARP_HWADDR_LEN;
    netif->mtu = 1500;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
}

/*********************************************************************
 * @fn      low_level_output
 *
 * @brief   Send one (possibly chained) pbuf as a single ethernet frame.
 */
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *q;
    u16_t len = p->tot_len;
    u8_t *dst;
    u16_t off = 0;

    (void)netif;

    if(len > ETH_MAX_PACKET_SIZE)
    {
        return ERR_MEM;
    }

    dst = ETH_GetTxBuf();
    if(dst == NULL)
    {
        /* Every TX descriptor is still owned by the DMA: wait for a TX-
         * complete interrupt to free one. The timeout only guards against
         * a dead/never-completing link, so TCP retransmission can recover. */
        if(xSemaphoreTake(eth_tx_semaphore, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            dst = ETH_GetTxBuf();
        }
        if(dst == NULL)
        {
            LINK_STATS_INC(link.drop);
            return ERR_IF;
        }
    }

    for(q = p; q != NULL; q = q->next)
    {
        MEMCPY(dst + off, q->payload, q->len);
        off += q->len;
    }

    ETH_SendFrame(len);

    LINK_STATS_INC(link.xmit);
    return ERR_OK;
}

/*********************************************************************
 * @fn      low_level_input
 *
 * @brief   Read one received frame from the DMA into a pool pbuf.
 *          Returns NULL when no frame is pending or on alloc failure.
 */
static struct pbuf *low_level_input(struct netif *netif)
{
    struct pbuf *p;
    u16_t len;
    u8_t *src;

    (void)netif;

    src = ETH_RecPeek(&len);
    if(src == NULL)
    {
        return NULL;
    }

    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if(p != NULL)
    {
        pbuf_take(p, src, len);
        LINK_STATS_INC(link.recv);
    }
    else
    {
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
    }

    /* give the descriptor back to the DMA in every case */
    ETH_RecFree();

    return p;
}

/*********************************************************************
 * @fn      ethernetif_input
 *
 * @brief   Block until the ETH ISR signals a received frame (or a short
 *          poll timeout expires so link/PHY handling keeps running), then
 *          push every pending frame into the tcpip_thread mailbox.
 *          Called from the netif task in netconf.c.
 */
void ethernetif_input(struct netif *netif)
{
    struct pbuf *p;

    xSemaphoreTake(eth_rx_semaphore, pdMS_TO_TICKS(10));

    /* Drain everything the DMA received since the last wake-up. */
    while((p = low_level_input(netif)) != NULL)
    {
        if(netif->input(p, netif) != ERR_OK)
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
        }
    }
}

/*********************************************************************
 * @fn      ethernetif_init
 *
 * @brief   netif init callback passed to netif_add().
 */
err_t ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
#if LWIP_ARP
    netif->output = etharp_output;
#endif /* LWIP_ARP */
    netif->linkoutput = low_level_output;

    low_level_init(netif);

    return ERR_OK;
}
