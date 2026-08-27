/**
 * @file
 * Ethernet netif for lwIP on CH32V307 (internal 10M PHY, raw DMA driver).
 *
 * RX is polled (no OS synchronisation inside the ISR): a dedicated
 * "netif" task calls ethernetif_input() and the frame is pushed into
 * the tcpip_thread mailbox via netif->input (tcpip_input).
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

#include "eth_driver.h"

#define IFNAME0 'e'
#define IFNAME1 'n'

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
        /* DMA still owns the TX descriptor: drop for now (TCP retries) */
        return ERR_IF;
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
 * @brief   Poll the interface and pass received frames to lwIP.
 *          Called repeatedly from the netif task in netconf.c.
 */
void ethernetif_input(struct netif *netif)
{
    struct pbuf *p;

    p = low_level_input(netif);
    if(p != NULL)
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
