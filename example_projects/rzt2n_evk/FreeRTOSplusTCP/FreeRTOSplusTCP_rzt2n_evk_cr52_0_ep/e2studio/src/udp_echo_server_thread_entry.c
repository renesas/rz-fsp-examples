/***********************************************************************************************************************
 * File Name    : udp_echo_server_thread.c
 * Description  : UDP echo server thread entry function.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "udp_echo_server_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

/* UDP Echo Server entry function */
uint8_t ucUdpReceivedBuffer[1500];

/*******************************************************************************************************************//**
 * @brief       UDP Echo Server thread entry function. Waits for the network to come up, then receives UDP datagrams
 *              and echoes each one back to its sender.
 * @param[in]   pvParameters    Contains TaskHandle_t (unused)
 * @retval      None
 **********************************************************************************************************************/
void udp_echo_server_thread_entry(void *pvParameters)
{
    Socket_t xUDPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t xClientLength = sizeof( struct freertos_sockaddr );
    int32_t lBytes;
    int32_t lResult;
    uint32_t xReceiveTimeout;

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Wait until the network interface is up before opening the socket. */
    while ((FreeRTOS_IsNetworkUp()) == (pdFALSE))
    {
        vTaskDelay (100);
    }

    /* Create the UDP socket. */
    xUDPSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
    if ((xUDPSocket) == (FREERTOS_INVALID_SOCKET))
    {
        vTaskDelete(NULL);
    }

    xServerAddress.sin_family = FREERTOS_AF_INET;
    /* Set the TCP server port number in network byte order. */
    xServerAddress.sin_port = FreeRTOS_htons( 10000 );

    /* Bind the socket to the server address. */
    lResult = FreeRTOS_bind( xUDPSocket, &xServerAddress, sizeof(xServerAddress) );
    if ((lResult) < (0))
    {
        vTaskDelete(NULL);
    }

    /* Configure the socket to block indefinitely while waiting for a datagram. */
    xReceiveTimeout = portMAX_DELAY;
    FreeRTOS_setsockopt( xUDPSocket,0,FREERTOS_SO_RCVTIMEO,&xReceiveTimeout,sizeof(xReceiveTimeout) );

    /* TODO: add your own code here */
    while (1)
    {
        /* Receive a datagram from a client. */
        lBytes = FreeRTOS_recvfrom( xUDPSocket,
                                    &ucUdpReceivedBuffer, sizeof( ucUdpReceivedBuffer ),
                                    0,
                                    &xClientAddress, &xClientLength );
        if ((lBytes) > (0))
        {
            /* Echo the received datagram back to the sender. */
            FreeRTOS_sendto(xUDPSocket,
                            &ucUdpReceivedBuffer, (size_t)lBytes,
                            0,
                            &xClientAddress, xClientLength );
        }
        else
        {
            vTaskDelete(NULL);
        }
    }
}
