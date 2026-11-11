/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "net_thread.h"
#include "FreeRTOS_IP.h"

static  uint8_t ucIPAddress[ 4 ]        = {192, 168, 1, 2};
static  uint8_t ucNetMask[ 4 ]          = {255, 255, 255, 0};
static  uint8_t ucGatewayAddress[ 4 ]   = {192, 168, 1, 1};
static  uint8_t ucDNSServerAddress[ 4 ] = {192, 168, 1, 1};

/* Net thread entry function */
/* pvParameters contains TaskHandle_t */
void net_thread_entry(void *pvParameters)
{
    BaseType_t status = pdFALSE;

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* FreeRTOS IP Initialization: This init initializes the IP stack  */
    status = FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, gp_freertos_ether->p_cfg->p_mac_address);
    if(pdFALSE == status)
    {
        vTaskDelete(NULL);
    }

    /* TODO: add your own code here */
    while (1)
    {
        vTaskDelay (100);
    }
}
