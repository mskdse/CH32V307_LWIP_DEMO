/********************************** (C) COPYRIGHT *******************************
* File Name          : main.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main program body.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*
 * @Note
 * FreeRTOS + lwIP: the "netconf" task calls LwIP_Init() which starts
 * tcpip_thread and an ethernet RX/link task. Network parameters are set in
 * lwipopts.h (LWIP_USE_DHCP / static IP / MAC).
 */
#include "string.h"
#include "debug.h"
#include "eth_driver.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/init.h"
#include "netconf.h"

/* Global define */
#define NETCONF_TASK_PRIO     5
#define NETCONF_STK_SIZE      384

/*********************************************************************
 * @fn      netconf_task
 *
 * @brief   Start lwIP (must run in task context, after the scheduler).
 */
static void netconf_task(void *pvParameters)
{
    (void)pvParameters;

    LwIP_Init();
    vTaskDelete(NULL);
}

/* configCHECK_FOR_STACK_OVERFLOW==1: FreeRTOS calls this on a task stack
 * overflow (e.g. if the ETH ISR, which runs on the interrupted task's stack,
 * overflows it). */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("STACK OVERFLOW: %s\r\n", pcTaskName);
    for(;;) { }
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemCoreClockUpdate();
	USART_Printf_Init(115200);

	printf("SystemClk:%d\r\n", SystemCoreClock);
	printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
	printf("FreeRTOS Kernel Version:%s\r\n", tskKERNEL_VERSION_NUMBER);
	printf("LwIP Version:%s\r\n", LWIP_VERSION_STRING);

	xTaskCreate((TaskFunction_t)netconf_task,
	            (const char *)"netconf",
	            (uint16_t)NETCONF_STK_SIZE,
	            (void *)NULL,
	            (UBaseType_t)NETCONF_TASK_PRIO,
	            (TaskHandle_t *)NULL);

	vTaskStartScheduler();

	for(;;) { }
}
