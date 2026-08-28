/********************************** (C) COPYRIGHT *******************************
* File Name          : eth_driver_RMII.c
* Description        : LAN8720A external PHY (RMII, 10/100M) driver for
*                      CH32V307 + FreeRTOS + lwIP.
*
* Selected instead of eth_driver_10M.c when LWIP_USE_LAN8720A == 1 (see
* eth_driver.h). It implements the SAME interface as eth_driver_10M.c, so the
* lwIP glue (netconf.c / ethernetif.c) is PHY-independent and unchanged:
*   ETH_Init / ETH_RecPeek / ETH_RecFree / ETH_GetTxBuf / ETH_SendFrame /
*   ETH_EnableMacFilter / WCHNET_HandlePhyNegotiation / WCHNET_TimeIsr /
*   WCHNET_GetMacAddr / LinkSta / MACRAW_Tx / ETH_IRQHandler
*
* Link status is polled via SMI in WCHNET_HandlePhyNegotiation(); the MAC
* speed/duplex is programmed from the LAN8720A PSCSR register on link-up.
*********************************************************************************/

#include "eth_driver.h"          /* defines LWIP_USE_LAN8720A */

#if LWIP_USE_LAN8720A

#include "string.h"
#include "ethernetif.h"
#include "FreeRTOS.h"
#include "task.h"

__attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];       /* MAC receive descriptor, 4-byte aligned */
__attribute__((__aligned__(4))) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];       /* MAC send descriptor, 4-byte aligned */

__attribute__((__aligned__(4))) uint8_t  MACRxBuf[ETH_RXBUFNB*ETH_RX_BUF_SZE];      /* MAC receive buffer, 4-byte aligned */
__attribute__((__aligned__(4))) uint8_t  MACTxBuf[ETH_TXBUFNB*ETH_TX_BUF_SZE];      /* MAC send buffer, 4-byte aligned */

uint16_t gPHYAddress;
uint32_t ChipId = 0;
uint32_t volatile LocalTime;
uint8_t volatile LinkSta = 0;   /* 0: no valid link   1: valid link */
ETH_DMADESCTypeDef *pDMARxSet;
ETH_DMADESCTypeDef *pDMATxSet;

/* link polling state (edge tracking so the MAC is only reconfigured on
 * link up/down transitions) */
static uint8_t phy_link_state = 0;      /* 0 = down, 1 = up */
static uint32_t link_poll_time = 0;     /* last SMI link poll, in LocalTime */

/*********************************************************************
 * @fn      WCHNET_GetMacAddr
 *
 * @brief   Read the unique MAC burned in the chip ROM.
 */
void WCHNET_GetMacAddr(uint8_t *p)
{
    uint8_t i;
    uint8_t *macaddr = (uint8_t *)(ROM_CFG_USERADR_ID + 5);

    for(i = 0; i < 6; i++)
    {
        *p = *macaddr;
        p++;
        macaddr--;
    }
}

/*********************************************************************
 * @fn      WCHNET_TimeIsr
 *
 * @brief   LocalTime base, fed by a FreeRTOS software timer (netconf.c).
 */
void WCHNET_TimeIsr(uint16_t timperiod)
{
    LocalTime += timperiod;
}

/*********************************************************************
 * @fn      ETH_RMIIPinInit
 *
 * @brief   LAN8720A RMII interface GPIO initialization.
 *          REFCLK (PA1) is an input: the PHY module supplies the 50MHz
 *          clock from its own crystal.
 */
static void ETH_RMIIPinInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
    GPIO_Output(GPIOA, GPIO_Pin_2);         /* MDIO */
    GPIO_Output(GPIOC, GPIO_Pin_1);         /* MDC  */
    GPIO_Output(GPIOB, GPIO_Pin_11);        /* TXEN */
    GPIO_Output(GPIOB, GPIO_Pin_12);        /* TXD0 */
    GPIO_Output(GPIOB, GPIO_Pin_13);        /* TXD1 */
    GPIO_Input(GPIOA, GPIO_Pin_1);          /* REFCLK */
    GPIO_Input(GPIOA, GPIO_Pin_7);          /* CRSDV */
    GPIO_Input(GPIOC, GPIO_Pin_4);          /* RXD0 */
    GPIO_Input(GPIOC, GPIO_Pin_5);          /* RXD1 */
}

/*********************************************************************
 * @fn      ETH_SetClock
 *
 * @brief   Ethernet MAC base clock (60MHz), identical to eth_driver_10M.c.
 *          Harmless when the RMII REFCLK is supplied by the PHY; keep it
 *          so both drivers share the same clock setup.
 */
static void ETH_SetClock(void)
{
    RCC_PLL3Cmd(DISABLE);
    RCC_PREDIV2Config(RCC_PREDIV2_Div2);                             /* HSE = 8M */
    RCC_PLL3Config(RCC_PLL3Mul_15);                                  /* 4M*15 = 60MHz */
    RCC_PLL3Cmd(ENABLE);
    while(RESET == RCC_GetFlagStatus(RCC_FLAG_PLL3RDY));
}

/*********************************************************************
 * @fn      ETH_RegInit
 *
 * @brief   MAC register config + LAN8720A reset + auto-negotiation start.
 *          Does NOT block until auto-negotiation finishes: that is handled
 *          asynchronously by WCHNET_HandlePhyNegotiation().
 */
static uint32_t ETH_RegInit(ETH_InitTypeDef *ETH_InitStruct, uint16_t PHYAddress)
{
    uint16_t timeout = 1000;

    /* SMI clock: HCLK/42 */
    ETH->MACMIIAR = (uint32_t)ETH_MACMIIAR_CR_Div42;

    /*------------------------ MAC register configuration ---------------------*/
    ETH->MACCR = (uint32_t)(ETH_InitStruct->ETH_Watchdog |
                  ETH_InitStruct->ETH_Jabber |
                  ETH_InitStruct->ETH_InterFrameGap |
                  ETH_InitStruct->ETH_ChecksumOffload |
                  ETH_InitStruct->ETH_AutomaticPadCRCStrip |
                  ETH_InitStruct->ETH_LoopbackMode
                  |(1 << 9));

    ETH->MACFFR = (uint32_t)(ETH_InitStruct->ETH_ReceiveAll |
                          ETH_InitStruct->ETH_SourceAddrFilter |
                          ETH_InitStruct->ETH_PassControlFrames |
                          ETH_InitStruct->ETH_BroadcastFramesReception |
                          ETH_InitStruct->ETH_DestinationAddrFilter |
                          ETH_InitStruct->ETH_PromiscuousMode |
                          ETH_InitStruct->ETH_MulticastFramesFilter |
                          ETH_InitStruct->ETH_UnicastFramesFilter);

    ETH->MACHTHR = (uint32_t)ETH_InitStruct->ETH_HashTableHigh;
    ETH->MACHTLR = (uint32_t)ETH_InitStruct->ETH_HashTableLow;

    ETH->MACFCR = (uint32_t)((ETH_InitStruct->ETH_PauseTime << 16) |
                     ETH_InitStruct->ETH_UnicastPauseFrameDetect |
                     ETH_InitStruct->ETH_ReceiveFlowControl |
                     ETH_InitStruct->ETH_TransmitFlowControl);

    ETH->MACVLANTR = (uint32_t)(ETH_InitStruct->ETH_VLANTagComparison |
                               ETH_InitStruct->ETH_VLANTagIdentifier);

    ETH->DMAOMR = (uint32_t)(ETH_InitStruct->ETH_DropTCPIPChecksumErrorFrame |
                    ETH_InitStruct->ETH_TransmitStoreForward |
                    ETH_InitStruct->ETH_ForwardErrorFrames |
                    ETH_InitStruct->ETH_ForwardUndersizedGoodFrames);

    /* Software reset the LAN8720A and wait (bounded) for the self-clear */
    ETH_WritePHYRegister(PHYAddress, PHY_BCR, PHY_Reset);
    do
    {
        vTaskDelay(10);
        if(!(--timeout)) break;
    } while(ETH_ReadPHYRegister(PHYAddress, PHY_BCR) & PHY_Reset);

    /* Mask all PHY interrupts (register 30); link status is polled instead */
    ETH_WritePHYRegister(PHYAddress, LAN8720A_CLEAR_PHY_REG, 0x00);

    /* Advertise 10BASE-T/100BASE-TX full+half duplex, then restart auto-neg */
    ETH_WritePHYRegister(PHYAddress, PHY_ANAR, 0x01E1);
    ETH_WritePHYRegister(PHYAddress, PHY_BCR,
                         PHY_AutoNegotiation | PHY_Restart_AutoNegotiation);

    return ETH_SUCCESS;
}

/*********************************************************************
 * @fn      ETH_Configuration
 *
 * @brief   Ethernet hardware configuration (RMII + LAN8720A).
 */
static void ETH_Configuration(uint8_t *macAddr)
{
    ETH_InitTypeDef ETH_InitStructure;
    uint16_t timeout = 10000;

    /* Enable Ethernet MAC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | \
                          RCC_AHBPeriph_ETH_MAC_Tx | \
                          RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);

    gPHYAddress = LAN8720A_PHY_ADDRESS;

    /* RMII GPIOs + MAC base clock */
    ETH_RMIIPinInit();
    ETH_SetClock();

    /* Reset ETHERNET on AHB Bus */
    ETH_DeInit();
    ETH_SoftwareReset();
    do
    {
        vTaskDelay(10);
        if(!(--timeout)) break;
    } while(ETH->DMABMR & ETH_DMABMR_SR);

    /*------------------------   MAC   -----------------------------------*/
    ETH_InitStructure.ETH_Watchdog = ETH_Watchdog_Enable;
    ETH_InitStructure.ETH_Jabber = ETH_Jabber_Enable;
    ETH_InitStructure.ETH_InterFrameGap = ETH_InterFrameGap_96Bit;
    ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Disable;
    ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
    ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;

    /* Filter function configuration */
    ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
    ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
    ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
    ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
    ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
    ETH_InitStructure.ETH_PassControlFrames = ETH_PassControlFrames_BlockAll;
    ETH_InitStructure.ETH_DestinationAddrFilter = ETH_DestinationAddrFilter_Normal;
    ETH_InitStructure.ETH_SourceAddrFilter = ETH_SourceAddrFilter_Disable;

    ETH_InitStructure.ETH_HashTableHigh = 0x0;
    ETH_InitStructure.ETH_HashTableLow = 0x0;

    /* VLan function configuration */
    ETH_InitStructure.ETH_VLANTagComparison = ETH_VLANTagComparison_16Bit;
    ETH_InitStructure.ETH_VLANTagIdentifier = 0x0;

    /* Flow Control function configuration */
    ETH_InitStructure.ETH_PauseTime = 0x0;
    ETH_InitStructure.ETH_UnicastPauseFrameDetect = ETH_UnicastPauseFrameDetect_Disable;
    ETH_InitStructure.ETH_ReceiveFlowControl = ETH_ReceiveFlowControl_Disable;
    ETH_InitStructure.ETH_TransmitFlowControl = ETH_TransmitFlowControl_Disable;

    /*------------------------   DMA   -----------------------------------*/
    /* Store and forward: the whole frame sits in the FIFO before the DMA
     * reads it, so the MAC can verify the checksum and drop bad frames. */
    ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable;
    ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;
    ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Enable;
    ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Enable;

    ETH_RegInit(&ETH_InitStructure, gPHYAddress);

    /* Configure MAC address */
    ETH->MACA0HR = (uint32_t)((macAddr[5]<<8) | macAddr[4]);
    ETH->MACA0LR = (uint32_t)(macAddr[0] | (macAddr[1]<<8) | (macAddr[2]<<16) | (macAddr[3]<<24));

    /* Mask the counter half-full interrupts */
    ETH->MMCTIMR = ETH_MMCTIMR_TGFM;
    ETH->MMCRIMR = ETH_MMCRIMR_RGUFM | ETH_MMCRIMR_RFCEM;

    /* RX is event-driven: the DMA R interrupt wakes the netif task via a
     * semaphore (ethernetif_rx_signal), and the T interrupt frees blocked
     * TX waits (ethernetif_tx_signal). */
    ETH_DMAITConfig(ETH_DMA_IT_NIS |\
                ETH_DMA_IT_AIS |\
                ETH_DMA_IT_RBU |\
                ETH_DMA_IT_R |\
                ETH_DMA_IT_T,\
                ENABLE);
}

/*********************************************************************
 * @fn      ETH_Stop
 *
 * @brief   Disable MAC reception/transmission (used on link-down).
 */
static void ETH_Stop(void)
{
    ETH_MACTransmissionCmd(DISABLE);
    ETH_FlushTransmitFIFO();
    ETH_MACReceptionCmd(DISABLE);
}

/*********************************************************************
 * @fn      ETH_RecPeek
 *
 * @brief   Non-blocking peek of the next valid received frame.
 *          Returns a pointer into the DMA RX buffer. The descriptor is
 *          NOT returned to the DMA until ETH_RecFree() is called, so the
 *          caller can safely copy the frame while DMA stays away.
 */
uint8_t *ETH_RecPeek(uint16_t *len)
{
    while(!(pDMARxSet->Status & ETH_DMARxDesc_OWN))
    {
        if(!(pDMARxSet->Status & ETH_DMARxDesc_ES) &&
           (pDMARxSet->Status & ETH_DMARxDesc_FS) &&
           (pDMARxSet->Status & ETH_DMARxDesc_LS))
        {
            *len = (uint16_t)((pDMARxSet->Status & ETH_DMARxDesc_FL) >> ETH_DMARxDesc_FrameLengthShift) - 4;
            return (uint8_t *)pDMARxSet->Buffer1Addr;
        }
        /* bad frame: discard it and move on */
        pDMARxSet->Status = ETH_DMARxDesc_OWN;
        pDMARxSet = (ETH_DMADESCTypeDef *)pDMARxSet->Buffer2NextDescAddr;
    }
    return NULL;
}

/*********************************************************************
 * @fn      ETH_RecFree
 *
 * @brief   Give the current RX descriptor (the one returned by
 *          ETH_RecPeek) back to the DMA and advance to the next one.
 */
void ETH_RecFree(void)
{
    pDMARxSet->Status = ETH_DMARxDesc_OWN;
    pDMARxSet = (ETH_DMADESCTypeDef *)pDMARxSet->Buffer2NextDescAddr;
    ETH->DMARPDR = 0;               /* resume DMA reception */
}

/*********************************************************************
 * @fn      ETH_GetTxBuf
 *
 * @brief   Get the current TX DMA buffer if its descriptor is free.
 */
uint8_t *ETH_GetTxBuf(void)
{
    if(DMATxDescToSet->Status & ETH_DMATxDesc_OWN)
    {
        return NULL;
    }
    return (uint8_t *)DMATxDescToSet->Buffer1Addr;
}

/*********************************************************************
 * @fn      ETH_SendFrame
 *
 * @brief   Commit a frame already copied into the buffer returned by
 *          ETH_GetTxBuf() and start DMA transmission.
 */
uint32_t ETH_SendFrame(uint16_t len)
{
    DMATxDescToSet->ControlBufferSize = (len & ETH_DMATxDesc_TBS1);
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;

    /* Set Own bit: give the buffer back to the ETHERNET DMA */
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;

    /* Clear TBUS flag and resume DMA transmission */
    ETH->DMASR = ETH_DMASR_TBUS;
    ETH->DMATPDR = 0;

    /* advance to the next TX descriptor (chained mode) */
    DMATxDescToSet = (ETH_DMADESCTypeDef *)DMATxDescToSet->Buffer2NextDescAddr;

    return ETH_SUCCESS;
}

/*********************************************************************
 * @fn      MACRAW_Tx
 *
 * @brief   Send a raw frame (not used by lwIP; kept for API parity with
 *          eth_driver_10M.c).
 */
uint32_t MACRAW_Tx(uint8_t *buff, uint16_t len)
{
    if((DMATxDescToSet->Status & ETH_DMATxDesc_OWN) != (u32)RESET)
    {
        return ETH_ERROR;
    }
    DMATxDescToSet->ControlBufferSize = (len & ETH_DMATxDesc_TBS1);
    memcpy((uint8_t *)DMATxDescToSet->Buffer1Addr, buff, len);
    DMATxDescToSet->Status |= ETH_DMATxDesc_LS | ETH_DMATxDesc_FS;
    DMATxDescToSet->Status |= ETH_DMATxDesc_OWN;
    ETH->DMASR = ETH_DMASR_TBUS;
    ETH->DMATPDR = 0;
    DMATxDescToSet = (ETH_DMADESCTypeDef *)DMATxDescToSet->Buffer2NextDescAddr;
    return ETH_SUCCESS;
}

/*********************************************************************
 * @fn      ETH_EnableMacFilter
 *
 * @brief   Restore normal MAC address filtering (never disabled in this
 *          driver; kept for interface parity with eth_driver_10M.c).
 */
void ETH_EnableMacFilter(void)
{
    ETH->MACFFR &= ~(ETH_ReceiveAll_Enable | ETH_PromiscuousMode_Enable);
}

/*********************************************************************
 * @fn      WCHNET_HandlePhyNegotiation
 *
 * @brief   Poll the LAN8720A link status (SMI) and keep LinkSta up to
 *          date. On a link-up edge the MAC speed/duplex is programmed from
 *          the PSCSR HCDSPEED field and RX/TX are enabled; on link-down
 *          they are disabled. Called from the netif task.
 */
void WCHNET_HandlePhyNegotiation(void)
{
    uint16_t bsr;

    /* throttle SMI link polling to PHY_LINK_TASK_PERIOD ms */
    if(LocalTime - link_poll_time < PHY_LINK_TASK_PERIOD)
    {
        return;
    }
    link_poll_time = LocalTime;

    /* BSR bit2 (link status) latches low: read twice for the live value */
    bsr = ETH_ReadPHYRegister(gPHYAddress, PHY_BSR);
    bsr = ETH_ReadPHYRegister(gPHYAddress, PHY_BSR);

    if(bsr & PHY_Linked_Status)
    {
        if(!phy_link_state)
        {
            uint16_t pscr = ETH_ReadPHYRegister(gPHYAddress, LAN8720A_PSCR_REG);

            phy_link_state = 1;
            /* PSCSR bits[4:2] HCDSPEED: 100M when bit3 set, full-duplex
             * when bit4 set (001=10HD 101=10FD 010=100HD 110=100FD). */
            if(pscr & (1 << 3))
                ETH->MACCR |= ETH_Speed_100M;
            else
                ETH->MACCR &= ~ETH_Speed_100M;
            if(pscr & (1 << 4))
                ETH->MACCR |= ETH_Mode_FullDuplex;
            else
                ETH->MACCR &= ~ETH_Mode_FullDuplex;
            ETH_Start();
        }
        LinkSta = 1;
    }
    else
    {
        if(phy_link_state)
        {
            phy_link_state = 0;
            ETH_Stop();
        }
        LinkSta = 0;
    }
}

/*********************************************************************
 * @fn      WCHNET_ETHIsr
 *
 * @brief   Ethernet DMA interrupt service routine body. Wakes the netif
 *          task / TX waiter via the ethernetif semaphores. Link status is
 *          polled (SMI), so there is no PHYLINK interrupt for this PHY.
 *
 * @note    Keep this ISR printf-free - it runs on the interrupted task's
 *          stack, and printf there can overflow small task stacks.
 */
static void WCHNET_ETHIsr(void)
{
    uint32_t int_sta;

    int_sta = ETH->DMASR;
    if(int_sta & ETH_DMA_IT_AIS)
    {
        if(int_sta & ETH_DMA_IT_RBU)
        {
            /* Receive buffer unavailable: resume DMA reception. If a free
             * descriptor exists the poll demand restarts the DMA; otherwise
             * it is a no-op and ETH_RecFree() resumes it later. */
            ETH->DMARPDR = 0;
            ETH_DMAClearITPendingBit(ETH_DMA_IT_RBU);
        }
        ETH_DMAClearITPendingBit(ETH_DMA_IT_AIS);
    }

    if(int_sta & ETH_DMA_IT_NIS)
    {
        if(int_sta & ETH_DMA_IT_R)
        {
            /* Wake the netif task: a frame has been received. */
            ethernetif_rx_signal();
            ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
        }
        if(int_sta & ETH_DMA_IT_T)
        {
            /* Wake the TX waiter: the DMA freed a TX descriptor. */
            ethernetif_tx_signal();
            ETH_DMAClearITPendingBit(ETH_DMA_IT_T);
        }
        ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    }
}

/*********************************************************************
 * @fn      ETH_Init
 *
 * @brief   Ethernet initialization (RMII + LAN8720A).
 */
void ETH_Init(uint8_t *macAddr)
{
    ChipId = DBGMCU_GetCHIPID();
    LinkSta = 0;
    phy_link_state = 0;

    /* let the LAN8720A power up before the first SMI access */
    vTaskDelay(20);

    ETH_Configuration(macAddr);
    ETH_DMATxDescChainInit(DMATxDscrTab, MACTxBuf, ETH_TXBUFNB);
    ETH_DMARxDescChainInit(DMARxDscrTab, MACRxBuf, ETH_RXBUFNB);
    pDMARxSet = DMARxDscrTab;
    pDMATxSet = DMATxDscrTab;

    /* poll the link immediately on the first loop iteration */
    link_poll_time = LocalTime - PHY_LINK_TASK_PERIOD;

    NVIC_EnableIRQ(ETH_IRQn);
    /* Same (lowest) priority as the FreeRTOS tick (0xf0): the ETH ISR and the
     * tick never nest, and both use software stack saving. */
    NVIC_SetPriority(ETH_IRQn, 0xf0);
}

/*********************************************************************
 * @fn      ETH_IRQHandler
 *
 * @brief   This function handles ETH exception.
 */
void ETH_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void ETH_IRQHandler(void)
{
    WCHNET_ETHIsr();
}

#endif /* LWIP_USE_LAN8720A */

/******************************** endfile @ eth_driver_RMII ******************************/
