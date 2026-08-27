/**
 * ethernetif.h - ethernet netif for lwIP on CH32V307 (raw DMA driver)
 *
 * OS-synchronisation glue between the ETH DMA interrupt (eth_driver_10M.c)
 * and the lwIP netif task:
 *   - ethernetif_sync_init() : create the RX/TX semaphores. Must be called
 *     before ETH_Init() so the ISR never signals a non-existent semaphore.
 *   - ethernetif_rx_signal() : called from the ETH ISR when a frame arrived.
 *   - ethernetif_tx_signal() : called from the ETH ISR when a TX completed.
 */
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#ifdef __cplusplus
extern "C" {
#endif

void ethernetif_sync_init(void);
void ethernetif_rx_signal(void);
void ethernetif_tx_signal(void);

#ifdef __cplusplus
}
#endif

#endif /* __ETHERNETIF_H__ */
