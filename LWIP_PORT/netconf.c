/**
 * netconf.c - network configuration / lwIP glue for CH32V307 + FreeRTOS
 *
 * Flow:
 *   LwIP_Init()  -> ETH_Init() (hardware) -> tcpip_init() -> netif_add()
 *   a "netif" task then polls RX frames, tracks link status (LinkSta,
 *   updated in the ETH ISR) and starts DHCP on the first link-up.
 *
 * The driver's PHY negotiation state machine is fed by a FreeRTOS software
 * timer (WCHNET_TimeIsr -> LocalTime), replacing the TIM2 hardware interrupt.
 */

#include "netconf.h"

#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "string.h"
#include "eth_driver.h"

/* from ethernetif.c */
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);

#define NETIF_TASK_PRIO    5
#define NETIF_TASK_STACK   512         /* words */

static struct netif g_netif;
static ip4_addr_t g_ip;
static ip4_addr_t g_mask;
static ip4_addr_t g_gw;

/* FreeRTOS software timer feeding the Ethernet driver's LocalTime base
 * (WCHNET_TimeIsr), so the driver's PHY negotiation can run. */
static TimerHandle_t localtime_timer;

static void localtime_timer_cb(TimerHandle_t t)
{
    (void)t;
    WCHNET_TimeIsr(WCHNETTIMERPERIOD);
}

/*********************************************************************
 * @fn      send_gratuitous_arp
 *
 * @brief   Broadcast a gratuitous ARP (who-has our own IP) so the peer
 *          learns our MAC/IP right after link-up.
 */
static void send_gratuitous_arp(struct netif *netif)
{
    u8_t frame[60];
    u8_t *mac = netif->hwaddr;
    const ip4_addr_t *ip = netif_ip4_addr(netif);

    memset(frame, 0, sizeof(frame));
    memset(frame, 0xff, 6);                     /* dest: broadcast */
    memcpy(frame + 6, mac, 6);                  /* src MAC */
    frame[12] = 0x08; frame[13] = 0x06;         /* ARP */
    frame[14] = 0x00; frame[15] = 0x01;         /* hw type ethernet */
    frame[16] = 0x08; frame[17] = 0x00;         /* proto IPv4 */
    frame[18] = 0x06; frame[19] = 0x04;         /* hw 6 proto 4 */
    frame[20] = 0x00; frame[21] = 0x01;         /* op: request */
    memcpy(frame + 22, mac, 6);                 /* sender MAC */
    memcpy(frame + 28, ip, 4);                  /* sender IP */
    memcpy(frame + 38, ip, 4);                  /* target IP = ours */
    memset(frame + 42, 0x55, sizeof(frame) - 42);

    MACRAW_Tx(frame, sizeof(frame));
}

/*********************************************************************
 * @fn      lwip_netif_thread
 *
 * @brief   RX poll + link management. Also drives the driver's PHY
 *          negotiation state machine.
 */
static void lwip_netif_thread(void *arg)
{
    struct netif *netif = (struct netif *)arg;
    u8_t link_was_up = 0;
#if LWIP_USE_DHCP
    u8_t dhcp_started = 0;
    u8_t ip_printed = 0;
#endif

    for(;;)
    {
        ethernetif_input(netif);
        /* driver PHY negotiation: corrects 10M PHY polarity / recovers a
         * dropped link; self-throttles on LocalTime (software timer) */
        WCHNET_HandlePhyNegotiation();

        if(LinkSta)
        {
            if(!link_was_up)
            {
                link_was_up = 1;
                netifapi_netif_set_link_up(netif);
                /* restore normal MAC filtering (driver enables promiscuous
                 * mode on link-up) */
                ETH_EnableMacFilter();
                send_gratuitous_arp(netif);
                printf("Link up\r\n");
#if LWIP_USE_DHCP
                if(!dhcp_started)
                {
                    dhcp_started = 1;
                    netifapi_dhcp_start(netif);
                }
#endif
            }
#if LWIP_USE_DHCP
            if(dhcp_started && netif_dhcp_data(netif) &&
               (netif_dhcp_data(netif)->state == DHCP_STATE_BOUND) && !ip_printed)
            {
                ip_printed = 1;
                printf("DHCP bound, IP:%s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
            }
#endif
        }
        else
        {
            if(link_was_up)
            {
                link_was_up = 0;
#if LWIP_USE_DHCP
                dhcp_started = 0;
                ip_printed = 0;
#endif
                netifapi_netif_set_link_down(netif);
                printf("Link down\r\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}

/*********************************************************************
 * @fn      LwIP_Init
 *
 * @brief   lwIP + hardware + netif init. Call from a task, after the
 *          scheduler has started.
 */
void LwIP_Init(void)
{
    u8_t mac[6];
    err_t err;

    /* 1. build the MAC address */
#if LWIP_USE_ROM_MAC
    WCHNET_GetMacAddr(mac);
#else
    mac[0] = LWIP_MAC_ADDR0;
    mac[1] = LWIP_MAC_ADDR1;
    mac[2] = LWIP_MAC_ADDR2;
    mac[3] = LWIP_MAC_ADDR3;
    mac[4] = LWIP_MAC_ADDR4;
    mac[5] = LWIP_MAC_ADDR5;
#endif

    /* 2. Ethernet hardware + DMA descriptors + ETH IRQ */
    ETH_Init(mac);

    /* 2b. LocalTime base via a FreeRTOS software timer (no TIM2 ISR) */
    localtime_timer = xTimerCreate("localtime",
                                   pdMS_TO_TICKS(WCHNETTIMERPERIOD),
                                   pdTRUE, NULL, localtime_timer_cb);
    if(localtime_timer != NULL)
    {
        xTimerStart(localtime_timer, 0);
    }

    /* 3. lwIP core (starts tcpip_thread) */
    tcpip_init(NULL, NULL);

    /* 4. addresses: DHCP -> 0.0.0.0, static -> from lwipopts.h */
#if LWIP_USE_DHCP
    ip4_addr_set_zero(&g_ip);
    ip4_addr_set_zero(&g_mask);
    ip4_addr_set_zero(&g_gw);
#else
    IP4_ADDR(&g_ip, LWIP_IPADDR0, LWIP_IPADDR1, LWIP_IPADDR2, LWIP_IPADDR3);
    IP4_ADDR(&g_mask, LWIP_NETMASK0, LWIP_NETMASK1, LWIP_NETMASK2, LWIP_NETMASK3);
    IP4_ADDR(&g_gw, LWIP_GATEWAY0, LWIP_GATEWAY1, LWIP_GATEWAY2, LWIP_GATEWAY3);
#endif

    /* 5. pre-fill MAC so low_level_init() can keep it */
    memcpy(&g_netif.hwaddr[0], mac, 6);

    err = netifapi_netif_add(&g_netif, &g_ip, &g_mask, &g_gw, NULL,
                             ethernetif_init, tcpip_input);
    if(err != ERR_OK)
    {
        printf("netif_add failed: %d\r\n", (int)err);
        return;
    }
    netifapi_netif_set_default(&g_netif);
    netifapi_netif_set_up(&g_netif);

    printf("Board IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(&g_netif)));
    printf("Board MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
           g_netif.hwaddr[0], g_netif.hwaddr[1], g_netif.hwaddr[2],
           g_netif.hwaddr[3], g_netif.hwaddr[4], g_netif.hwaddr[5]);

    /* 6. RX / link management task */
    if(xTaskCreate(lwip_netif_thread, "netif", NETIF_TASK_STACK,
                   &g_netif, NETIF_TASK_PRIO, NULL) != pdPASS)
    {
        printf("netif task create failed\r\n");
    }

    printf("LwIP init done\r\n");
}

/*********************************************************************
 * @fn      LwIP_GetNetif
 */
struct netif *LwIP_GetNetif(void)
{
    return &g_netif;
}

/*********************************************************************
 * @fn      LwIP_DHCP_Start
 */
err_t LwIP_DHCP_Start(void)
{
    return netifapi_dhcp_start(&g_netif);
}

/*********************************************************************
 * @fn      LwIP_DHCP_Stop
 */
err_t LwIP_DHCP_Stop(void)
{
    return netifapi_dhcp_release_and_stop(&g_netif);
}

/*********************************************************************
 * @fn      LwIP_DHCP_GetState
 */
u8_t LwIP_DHCP_GetState(void)
{
#if LWIP_DHCP
    if(netif_dhcp_data(&g_netif))
    {
        return netif_dhcp_data(&g_netif)->state;
    }
#endif
    return 0;
}

/*********************************************************************
 * @fn      LwIP_GetIPAddr
 */
void LwIP_GetIPAddr(u8_t *ip)
{
    const ip4_addr_t *a = netif_ip4_addr(&g_netif);

    ip[0] = ip4_addr1(a);
    ip[1] = ip4_addr2(a);
    ip[2] = ip4_addr3(a);
    ip[3] = ip4_addr4(a);
}

/*********************************************************************
 * @fn      LwIP_GetMACAddr
 */
void LwIP_GetMACAddr(u8_t *mac)
{
    memcpy(mac, g_netif.hwaddr, 6);
}
