/***********************************************************************************************************************
 * File Name    : udp_echo_server_thread_entry.c
 * Description  : Contains declarations of data structures and functions used in udp_echo_server_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "udp_echo_server_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "common_utils.h"
#include <ctype.h>

/***********************************************************************************************************************
 * @addtogroup rm_FreeRTOSplusTCP_ep
 * @{
 **********************************************************************************************************************/

uint8_t g_ucUdpReceivedBuffer[1500];

/***********************************************************************************************************************
 * @brief       This function prints an IPv4 address in dotted-decimal format.
 *              The input address is expected to be in network byte order.
 * @param[in]   addr_be         IPv4 address in big-endian format.
 * @retval      None.
 **********************************************************************************************************************/
static void print_ipv4_addr(uint32_t addr_be)
{
    uint32_t ip = FreeRTOS_ntohl(addr_be);

    APP_PRINT("%u.%u.%u.%u",
              (unsigned int)((ip >> 24) & 0xFFU),
              (unsigned int)((ip >> 16) & 0xFFU),
              (unsigned int)((ip >> 8) & 0xFFU),
              (unsigned int)(ip & 0xFFU));
}

/***********************************************************************************************************************
 * @brief       This function prints the received UDP payload as ASCII text for debug purposes.
 *              Non-printable characters are replaced with '.' and the output length is limited.
 * @param[in]   p_data          Pointer to payload buffer.
 * @param[in]   length          Number of bytes in the payload.
 * @retval      None.
 **********************************************************************************************************************/
static void print_udp_payload_ascii(const uint8_t * p_data, BaseType_t length)
{
    BaseType_t i;
    BaseType_t print_len = length;

    if (print_len > 64)
    {
        print_len = 64;
    }

    APP_PRINT("ASCII: \"");
    for (i = 0; i < print_len; i++)
    {
        uint8_t ch = p_data[i];
        APP_PRINT("%c", isprint((int) ch) ? ch : '.');
    }
    if (length > print_len)
    {
        APP_PRINT("...");
    }
    APP_PRINT("\"\r\n");
}

/***********************************************************************************************************************
 * @brief       This function runs a simple UDP echo server on port 10000.
 *              It waits for the network to become available, receives UDP datagrams from a client, and sends
 *              the same payload back to the sender while printing debug information.
 * @param[in]   pvParameters    Pointer to task parameters (unused).
 * @retval      None.
 **********************************************************************************************************************/
void udp_echo_server_thread_entry(void *pvParameters)
{
    Socket_t xUDPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t xClientLength = sizeof(struct freertos_sockaddr);
    BaseType_t lBytes;
    BaseType_t lResult;
    TickType_t xReceiveTimeout;

    FSP_PARAMETER_NOT_USED (pvParameters);
    while (FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay (100);
    }

    APP_PRINT("UDP echo server: network is up, starting...\r\n");

    /* Create a UDP socket for the echo server */
    xUDPSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);

    /* Handle error */
    if (FREERTOS_INVALID_SOCKET == xUDPSocket)
    {
        APP_PRINT("FreeRTOS_socket for UDP socket failed\r\n");
        vTaskDelete(NULL);
    }

    /* Bind the UDP socket to local port 10000 */
    xServerAddress.sin_family = FREERTOS_AF_INET;
    xServerAddress.sin_port = FreeRTOS_htons(10000);
    lResult = FreeRTOS_bind(xUDPSocket, &xServerAddress, sizeof(xServerAddress));

    /* Handle error */
    if (lResult < 0)
    {
        APP_PRINT("FreeRTOS_bind for UDP socket failed, ret=%d\r\n", (int)lResult);
        FreeRTOS_closesocket(xUDPSocket);
        vTaskDelete(NULL);
    }

    /* Configure the receive timeout for the UDP socket */
    xReceiveTimeout = portMAX_DELAY;

    /* Receive UDP datagrams and echo them back to the sender */
    FreeRTOS_setsockopt(xUDPSocket ,
                        0,
                        FREERTOS_SO_RCVTIMEO,
                        &xReceiveTimeout,
                        sizeof(xReceiveTimeout));
    APP_PRINT("UDP echo server started on port 10000\r\n");

    /* Main UDP server loop: wait for incoming datagrams and echo the received payload back to the sender */
    while (1)
    {
        xClientLength = sizeof(struct freertos_sockaddr);

        lBytes = FreeRTOS_recvfrom(xUDPSocket,
                                    &g_ucUdpReceivedBuffer,
                                    sizeof(g_ucUdpReceivedBuffer),
                                    0,
                                    &xClientAddress,
                                    &xClientLength);

        if (lBytes > 0)
        {
            APP_PRINT("[UDP RX] from ");
            print_ipv4_addr(xClientAddress.sin_addr);
            APP_PRINT(":%u -> local:%u, len=%d\r\n",
                        (unsigned int) FreeRTOS_ntohs(xClientAddress.sin_port),
                        10000U,
                        (int) lBytes);

            print_udp_payload_ascii(g_ucUdpReceivedBuffer, lBytes);

            lResult = FreeRTOS_sendto(xUDPSocket,
                                        &g_ucUdpReceivedBuffer,
                                        (size_t)lBytes,
                                        0,
                                        &xClientAddress,
                                        xClientLength);

            if (lResult < 0)
            {
                APP_PRINT("[UDP TX] echo failed, ret=%d\r\n", (int)lResult);
            }
            else
            {
                APP_PRINT("[UDP TX] echo success, sent=%d bytes to ", (int)lResult);
                print_ipv4_addr(xClientAddress.sin_addr);
                APP_PRINT(":%u\r\n", (unsigned int)FreeRTOS_ntohs(xClientAddress.sin_port));
            }
        }
        else if (0 == lBytes)
        {
            APP_PRINT("[UDP] No data received\r\n");
        }
        else
        {
            APP_PRINT("[UDP] Receive failed, ret=%d\r\n", (int)lBytes);
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup rm_FreeRTOSplusTCP_ep)
 **********************************************************************************************************************/
