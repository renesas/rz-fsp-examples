/***********************************************************************************************************************
 * File Name    : tcp_echo_server_thread_entry.c
 * Description  : Contains declarations of data structures and functions used in tcp_echo_server_thread_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2024 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "tcp_echo_server_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "common_utils.h"
#include <ctype.h>

/***********************************************************************************************************************
 * @addtogroup rm_FreeRTOSplusTCP_ep
 * @{
 **********************************************************************************************************************/

/* Buffer used to store received TCP payload before echoing it back */
uint8_t g_ucTcpReceivedBuffer[1500];

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
 * @brief       Prints the received TCP payload as ASCII text for debug purposes.
 *              Non-printable characters are replaced with '.' and the output length is limited.
 * @param[in]   p_data          Pointer to payload buffer.
 * @param[in]   length          Number of bytes in the payload.
 * @retval      None.
 **********************************************************************************************************************/
static void print_tcp_payload_ascii(const uint8_t * p_data, BaseType_t length)
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
        APP_PRINT("%c", isprint((int)ch) ? ch : '.');
    }
    if (length > print_len)
    {
        APP_PRINT("...");
    }
    APP_PRINT("\"\r\n");
}

/***********************************************************************************************************************
 * @brief       This function runs a simple TCP echo server on port 10000.
 *              It waits for the network to become available, accepts incoming TCP connections, receives data
 *              from a client, and echoes the same payload back to the sender while printing debug information.
 * @param[in]   pvParameters    Pointer to task parameters (unused).
 * @retval      None
 **********************************************************************************************************************/
void tcp_echo_server_thread_entry(void *pvParameters)
{
    Socket_t xTCPListenSocket;
    Socket_t xTCPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t xClientLength = sizeof(struct freertos_sockaddr);
    BaseType_t lBytes;
    BaseType_t lResult;
    TickType_t xReceiveTimeout;
    BaseType_t xBacklog;

    FSP_PARAMETER_NOT_USED (pvParameters);

    while (FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay (100);
    }

    APP_PRINT("TCP echo server: network is up, starting...\r\n");

    /* Create a TCP socket for the echo server */
    xTCPListenSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);

    /* Handle error */
    if (FREERTOS_INVALID_SOCKET == xTCPListenSocket)
    {
        APP_PRINT("TCP socket creation failed\r\n");
        vTaskDelete(NULL);
    }

    /* Bind the TCP socket to local port 10000 */
    xServerAddress.sin_family = FREERTOS_AF_INET;
    xServerAddress.sin_port = FreeRTOS_htons(10000);
    lResult = FreeRTOS_bind(xTCPListenSocket, &xServerAddress, sizeof(xServerAddress));

    /* Handle error */
    if (lResult < 0)
    {
        APP_PRINT("FreeRTOS_bind for TCP socket failed, ret=%d\r\n", (int)lResult);
        FreeRTOS_closesocket(xTCPListenSocket);
        vTaskDelete(NULL);
    }

    /* Configure the receive timeout for the listening socket */
    xReceiveTimeout = portMAX_DELAY;

    /* Put the socket into listening state */
    FreeRTOS_setsockopt(xTCPListenSocket,
                        0,
                        FREERTOS_SO_RCVTIMEO,
                        &xReceiveTimeout,
                        sizeof(xReceiveTimeout) );

    xBacklog = 10;
    FreeRTOS_listen(xTCPListenSocket, xBacklog);
    APP_PRINT("TCP echo server started on port 10000\r\n");

    /* Main TCP server loop: wait for client connections, receive TCP data,
     * and echo the payload back until the session ends */
    while (1)
    {
        xClientLength = sizeof(struct freertos_sockaddr);
        xTCPSocket = FreeRTOS_accept(xTCPListenSocket,
                                        &xClientAddress, &xClientLength);

        /* Handle error */
        if (FREERTOS_INVALID_SOCKET == xTCPSocket)
        {
            APP_PRINT("TCP accept failed\r\n");
            FreeRTOS_closesocket(xTCPListenSocket);
            vTaskDelete(NULL);
        }
        else
        {
            APP_PRINT("[TCP] client connected from ");
            print_ipv4_addr(xClientAddress.sin_addr);
            APP_PRINT(":%u\r\n", (unsigned int)FreeRTOS_ntohs(xClientAddress.sin_port));
            xReceiveTimeout = portMAX_DELAY;
            FreeRTOS_setsockopt(xTCPSocket ,
                                0,
                                FREERTOS_SO_RCVTIMEO,
                                &xReceiveTimeout,
                                sizeof(xReceiveTimeout) );
            do
            {
                lBytes = FreeRTOS_recv(xTCPSocket,
                                        &g_ucTcpReceivedBuffer,
                                        sizeof(g_ucTcpReceivedBuffer),
                                        0);

                if (lBytes > 0)
                {
                    APP_PRINT("[TCP RX] from ");
                    print_ipv4_addr(xClientAddress.sin_addr);
                    APP_PRINT(":%u -> local:%u, len=%d\r\n",
                                (unsigned int) FreeRTOS_ntohs(xClientAddress.sin_port),
                                10000U,
                                (int)lBytes);

                    print_tcp_payload_ascii(g_ucTcpReceivedBuffer, lBytes);

                    lResult = FreeRTOS_send(xTCPSocket,
                                            &g_ucTcpReceivedBuffer,
                                            (size_t)lBytes,
                                            0);

                    if (lResult < 0)
                    {
                        APP_PRINT("[TCP TX] echo failed, ret=%d\r\n", (int)lResult);
                        break;
                    }
                    else
                    {
                        APP_PRINT("[TCP TX] echo success, sent=%d bytes\r\n", (int)lResult);
                    }
                }
                else if (0 == lBytes)
                {
                    APP_PRINT("[TCP] client disconnected normally\r\n");
                }
                else
                {
                    APP_PRINT("[TCP] session closed, ret=%d\r\n", (int)lBytes);
                }
            } while (lBytes > 0);

            FreeRTOS_closesocket(xTCPSocket);
            APP_PRINT("[TCP] socket closed\r\n");
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup rm_FreeRTOSplusTCP_ep)
 **********************************************************************************************************************/
