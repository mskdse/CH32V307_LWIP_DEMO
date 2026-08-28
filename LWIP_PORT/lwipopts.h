/**
 * lwipopts.h - lwIP configuration for CH32V307 + FreeRTOS
 * Target: lwIP static RAM footprint < 15KB
 *
 * Network parameters (IP / netmask / gateway / MAC / DHCP) are all
 * configured at the bottom of this file.
 */

#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <stdint.h>

/* ====================================================
 * 1. Operating system (FreeRTOS, full OS mode)
 * ==================================================== */
#define NO_SYS                          0               /* 0: use an OS (tcpip_thread) */
#define SYS_LIGHTWEIGHT_PROT            1
#define LWIP_NETCONN                    1               /* netconn API on */
#define LWIP_NETIF_API                  1               /* netifapi_* usable from tasks (netconf needs it) */
#define LWIP_SOCKET                     0               /* socket API off (saves RAM) */
#define LWIP_COMPAT_SOCKETS             0
#define LWIP_POSIX_SOCKETS_IO_NAMES     0

/* ====================================================
 * 2. Protocol switches (only what is needed)
 * ==================================================== */
#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_IPV4                       1
#define LWIP_IPV6                       0
#define LWIP_ICMP                       1               /* ping */
#define LWIP_RAW                        0
#define LWIP_UDP                        1
#define LWIP_TCP                        1
#define LWIP_DNS                        0
#define LWIP_DHCP                       1               /* DHCP support (0 = static only) */
#define LWIP_AUTOIP                     0
#define LWIP_SNMP                       0
#define LWIP_IGMP                       0
#define LWIP_MULTICAST_TX_OPTIONS       0

/* ====================================================
 * 3. Memory configuration
 * ==================================================== */
#define MEM_ALIGNMENT                   4

#define MEM_SIZE                        (6 * 1024)      /* heap for DHCP / small allocs */

#define PBUF_POOL_SIZE                  10              /* pool pbufs */
#define PBUF_POOL_BUFSIZE               512             /* 1514B frame -> 3 chained pbufs */

#define MEMP_NUM_PBUF                   10
#define MEMP_NUM_RAW_PCB                0
#define MEMP_NUM_UDP_PCB                6               /* DHCP uses one */
#define MEMP_NUM_TCP_PCB                6
#define MEMP_NUM_TCP_PCB_LISTEN         2
#define MEMP_NUM_TCP_SEG                12
#define MEMP_NUM_NETCONN                6
#define MEMP_NUM_NETBUF                 6
#define MEMP_NUM_SYS_TIMEOUT            12              /* DHCP(2)+ARP(1)+tcp_tmr(1)+4xTCP retrans */
#define MEMP_NUM_ARP_QUEUE              12
#define MEMP_NUM_TCPIP_MSG_INPKT        12              /* RX mailbox capacity */
#define MEMP_NUM_TCPIP_MSG_API          8

#define ARP_TABLE_SIZE                  10

#define IP_REASSEMBLY                   0
#define IP_FRAG                         0
#define LWIP_NETIF_HOSTNAME             0

/* ====================================================
 * 4. TCP
 * ==================================================== */
#define TCP_MSS                         512
#define TCP_WND                         (4 * TCP_MSS)
#define TCP_SND_BUF                     (4 * TCP_MSS)
#define TCP_SND_QUEUELEN                8
#define TCP_LISTEN_BACKLOG              0
#define TCP_OVERSIZE                    TCP_MSS
#define TCP_SYN_RETRIES                 3
#define TCP_MAXRTX                      4
#define TCP_RETRANSMISSION_TIMEOUT      3000

/* tcpip_thread stack (bytes) and priority (FreeRTOS prio) */
#define TCPIP_THREAD_STACKSIZE          1536            /* -> 384 words */
#define TCPIP_THREAD_PRIO               5               /* equal to all other app tasks for debugging */
#define TCPIP_MBOX_SIZE                 16

/* ====================================================
 * 5. UDP / ICMP
 * ==================================================== */
#define UDP_TTL                         255
#define LWIP_UDP_CHKSUM                 1
#define ICMP_TTL                        255

/* ====================================================
 * 6. ARP
 * ==================================================== */
#define ARP_QUEUEING                    1
#define ETHARP_SUPPORT_STATIC_ENTRIES   0

/* ====================================================
 * 7. DHCP
 * ==================================================== */
#define DHCP_DOES_ARP_CHECK             1

/* ====================================================
 * 8. Stats / debug (all off)
 * ==================================================== */
#define LWIP_STATS                      0
#define LWIP_DEBUG                      0

/* ====================================================
 * 9. OS / timeout integration
 * ==================================================== */
#define LWIP_TCPIP_CORE_LOCKING         1
#define LWIP_TCPIP_CORE_LOCKING_INPUT   0               /* RX via tcpip mailbox (thread-safe from any task) */
#define LWIP_TCPIP_TIMEOUT              1

/* ====================================================
 * 10. Checksums (software; MAC offload disabled in driver)
 * ==================================================== */
#define LWIP_CHECKSUM_CTRL_PER_NETIF    1
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define CHECKSUM_CHECK_ICMP             1

#define LWIP_PROVIDE_ERRNO              0
#define LWIP_TIMEVAL_PRIVATE            1

/* ====================================================
 * 11. NETWORK PARAMETERS (edit here)
 * ==================================================== */
/* Ethernet PHY: 1 = external LAN8720A (RMII 10/100M), 0 = internal 10M PHY.
 * This switch lives in ETH_Driver/eth_driver.h (LWIP_USE_LAN8720A); it only
 * decides which of the two driver files compiles, the lwIP layer is unchanged. */

/* 1: DHCP dynamic address, 0: static address below */
#define LWIP_USE_DHCP                   0

/* 1: use the unique MAC burned in the chip ROM, 0: use LWIP_MAC_ADDRn */
#define LWIP_USE_ROM_MAC                0

#if (LWIP_USE_ROM_MAC == 0)
#define LWIP_MAC_ADDR0                  0x04
#define LWIP_MAC_ADDR1                  0xBF
#define LWIP_MAC_ADDR2                  0x1b
#define LWIP_MAC_ADDR3                  0x11
#define LWIP_MAC_ADDR4                  0x22
#define LWIP_MAC_ADDR5                  0x33
#endif

/* Static IPv4 (used when LWIP_USE_DHCP == 0) */
#define LWIP_IPADDR0                    169
#define LWIP_IPADDR1                    254
#define LWIP_IPADDR2                    231
#define LWIP_IPADDR3                    0

#define LWIP_NETMASK0                   255
#define LWIP_NETMASK1                   255
#define LWIP_NETMASK2                   0
#define LWIP_NETMASK3                   0

#define LWIP_GATEWAY0                   169
#define LWIP_GATEWAY1                   254
#define LWIP_GATEWAY2                   231
#define LWIP_GATEWAY3                   203

/* ====================================================
 * 12. Approximate lwIP static RAM footprint
 * ----------------------------------------------------
 * MEM heap         4.0 KB
 * PBUF pool        8 * ~520B       ~4.2 KB
 * memp pools        (ARP que 10, TCP seg 8)  ~3.0 KB
 * ARP table        8 entries       ~0.3 KB
 * ----------------------------------------------------
 * total            ~11.5 KB (< 15 KB target)
 * ==================================================== */

#endif /* __LWIPOPTS_H__ */
