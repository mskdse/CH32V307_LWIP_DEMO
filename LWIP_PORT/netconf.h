/**
 * netconf.h - network configuration / lwIP glue for CH32V307 + FreeRTOS
 *
 * Network parameters (IP / netmask / gateway / MAC / DHCP switch) are
 * configured in lwipopts.h.
 */
#ifndef __NETCONF_H__
#define __NETCONF_H__

#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize lwIP, add the ethernet netif and start the RX/link task.
 *  Must be called from a task context AFTER vTaskStartScheduler(). */
void LwIP_Init(void);

/** Return the global netif. */
struct netif *LwIP_GetNetif(void);

/** Start DHCP (manual; in DHCP mode it is also started automatically
 *  on the first link-up). */
err_t LwIP_DHCP_Start(void);

/** Release the address and stop DHCP. */
err_t LwIP_DHCP_Stop(void);

/** Return the DHCP state (0 when not started / DHCP disabled). */
u8_t LwIP_DHCP_GetState(void);

/** Copy the current IPv4 address into ip[4]. */
void LwIP_GetIPAddr(u8_t *ip);

/** Copy the MAC address used by the netif into mac[6]. */
void LwIP_GetMACAddr(u8_t *mac);

#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H__ */
