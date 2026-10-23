/***********************************************************************************************************************
 * File Name    : net_thread_entry.c
 * Description  : FreeRTOS+TCP network initialization thread entry function.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "net_thread.h"
#include "FreeRTOS_IP.h"
#include "common_utils.h"

static uint8_t ucIPAddress[4]        = {192, 168, 0, 100};
static uint8_t ucNetMask[4]          = {255, 255, 255, 0};
static uint8_t ucGatewayAddress[4]   = {192, 168, 0, 3};
static uint8_t ucDNSServerAddress[4] = {10, 60, 1, 2};

/*******************************************************************************************************************//**
 * @brief       Net Thread entry function. Initializes the FreeRTOS+TCP IP stack with the static network
 *              configuration before printing the assigned addresses.
 * @param[in]   pvParameters    Contains TaskHandle_t (unused)
 * @retval      None
 **********************************************************************************************************************/
void net_thread_entry(void *pvParameters)
{
    BaseType_t status = pdFALSE;
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* FreeRTOS IP Initialization: This init initializes the IP stack  */
    /* Must run first - the network must not depend on the UART console */
    status = FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress,
                              gp_freertos_ether->p_cfg->p_mac_address);
    if ((pdFALSE) == (status))
    {
        vTaskDelete(NULL);
    }

    /* Print the banner and the example project overview */
    APP_PRINT(BANNER_INFO, EP_VERSION, 4, 2, 0);
    APP_PRINT("\r\nExample Project Overview\r\n");
    APP_PRINT("This example demonstrates TCP/UDP echo server communication using FreeRTOS+TCP.\r\n");
    APP_PRINT("\r\n");

    APP_PRINT("IP Address     : %d.%d.%d.%d\r\n",ucIPAddress[0], ucIPAddress[1], ucIPAddress[2], ucIPAddress[3]);
    APP_PRINT("Subnet Mask    : %d.%d.%d.%d\r\n",ucNetMask[0], ucNetMask[1], ucNetMask[2], ucNetMask[3]);
    APP_PRINT("Gateway        : %d.%d.%d.%d\r\n",
              ucGatewayAddress[0], ucGatewayAddress[1], ucGatewayAddress[2], ucGatewayAddress[3]);
    APP_PRINT("\r\n");

    /* TODO: add your own code here */
    while (1)
    {
        vTaskDelay (100);
    }
}
