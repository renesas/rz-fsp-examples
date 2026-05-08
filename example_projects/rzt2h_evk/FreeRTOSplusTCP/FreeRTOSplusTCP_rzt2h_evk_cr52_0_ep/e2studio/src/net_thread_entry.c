/***********************************************************************************************************************
 * File Name    : net_thread_entry.c
 * Description  : Contains declarations of data structures and functions used in net_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "net_thread.h"
#include "FreeRTOS_IP.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * @addtogroup rm_FreeRTOSplusTCP_ep
 * @{
 **********************************************************************************************************************/

/* Number of Ethernet switch ports monitored by this example */
#define PING_PORT_COUNT    (3U)

/* Link monitor state variables */
static uint8_t s_link_status = 0U;
static uint8_t s_link_change = 0U;
static BaseType_t s_network_up_printed = pdFALSE;

/* Static IPv4 network configuration used by FreeRTOS+TCP */
static uint8_t ucIPAddress[4]        = {192, 168, 0, 100};
static uint8_t ucNetMask[4]          = {255, 255, 255, 0};
static uint8_t ucGatewayAddress[4]   = {192, 168, 0, 3};
static uint8_t ucDNSServerAddress[4] = {10, 60, 1, 2};

/***********************************************************************************************************************
 * @brief       This function checks link status of all monitored Ethernet ports and prints a message
 *              when a port changes state. Polls the GMAC driver, updates the local link bitmask,
 *              and reports link up/down events.
 * @param[in]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void check_link_status(void)
{
    fsp_err_t            err = FSP_SUCCESS;
    gmac_b_link_status_t port_status;
    uint8_t              port;
    uint8_t              port_bit;
    uint8_t              previous_link_status = s_link_status;

    s_link_status = 0U;

    for (port = 0; port < PING_PORT_COUNT; port++)
    {
        err = R_GMAC_B_GetLinkStatus(&g_ether0_ctrl, port, &port_status);
        if (FSP_SUCCESS != err)
        {
            APP_PRINT("R_GMAC_B_GetLinkStatus(port=%u) failed, err=%d\r\n", port, err);
            return;
        }

        if (GMAC_LINK_STATUS_DOWN != port_status)
        {
            s_link_status |= (uint8_t) (1U << port); /* Cast to unsigned integer */
        }
    }

    s_link_change = (uint8_t) (previous_link_status ^ s_link_status); /* Cast to unsigned integer */

    for (port = 0; port < PING_PORT_COUNT; port++)
    {
        port_bit = (uint8_t) (1U << port); /* Cast to unsigned integer */

        if (s_link_change & port_bit)
        {
            s_link_change &= (uint8_t) (~port_bit); /* Cast to unsigned integer */

            if (s_link_status & port_bit)
            {
                APP_PRINT("Port %u link is up\r\n", port);
            }
            else
            {
                APP_PRINT("Port %u link is down\r\n", port);
            }
        }
    }
}

/***********************************************************************************************************************
 * @brief       This function initializes the FreeRTOS+TCP stack and monitors network/link status.
 *              It prints example information, configures the static IPv4 settings, and periodically checks
 *              whether the network stack and Ethernet links are up.
 * @param[in]   pvParameters    Pointer to task parameters (unused).
 * @retval      None.
 **********************************************************************************************************************/
void net_thread_entry(void *pvParameters)
{
    BaseType_t status = pdFALSE;
    fsp_pack_version_t version = {RESET_VALUE};
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Get the FSP version */
    R_FSP_VersionGet(&version);

    /* Example project information printed on the console */
    APP_PRINT(BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor,
            version.version_id_b.patch);
    APP_PRINT(EP_INFO);

    /* Print current static IPv4 network configuration */
    APP_PRINT("Board IP          : %u.%u.%u.%u\r\n",
                ucIPAddress[0], ucIPAddress[1], ucIPAddress[2], ucIPAddress[3]);
    APP_PRINT("Netmask           : %u.%u.%u.%u\r\n",
                ucNetMask[0], ucNetMask[1], ucNetMask[2], ucNetMask[3]);
    APP_PRINT("Gateway           : %u.%u.%u.%u\r\n",
                ucGatewayAddress[0], ucGatewayAddress[1], ucGatewayAddress[2], ucGatewayAddress[3]);
    APP_PRINT("UDP/TCP echo port : 10000\r\n\r\n");

    /* FreeRTOS IP Initialization: This initializes the IP stack */
    status = FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress,
                             gp_freertos_ether->p_cfg->p_mac_address);

    /* Handle error */
    if (pdFALSE == status)
    {
        APP_PRINT("FreeRTOS_IPInit API failed\r\n");
        vTaskDelete(NULL);
    }
    else
    {
        APP_PRINT("FreeRTOS_IPInit API success, waiting for network up...\r\n");
    }

    while (1)
    {
        check_link_status();
        if ((pdFALSE == s_network_up_printed) && (pdFALSE != FreeRTOS_IsNetworkUp()))
        {
            s_network_up_printed = pdTRUE;
            APP_PRINT("Network is up, board is reachable at %u.%u.%u.%u\r\n",
                        ucIPAddress[0], ucIPAddress[1], ucIPAddress[2], ucIPAddress[3]);
        }
        vTaskDelay(100);
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup rm_FreeRTOSplusTCP_ep)
 **********************************************************************************************************************/
