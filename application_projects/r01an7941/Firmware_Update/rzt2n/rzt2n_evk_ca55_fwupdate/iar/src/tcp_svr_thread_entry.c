/***********************************************************************************************************************
 * Copyright [2020-2024] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "tcp_svr_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "fwupdate.h"

/******************************************************************************
 * @fn       tcp_svr_thread_entry
 * @brief    TCPSvr entry function.
 *
 * @param[in]  pvParameters    Task processing parameters.
 ******************************************************************************/
void tcp_svr_thread_entry(void * pvParameters)
{
    Socket_t                 xTCPListenSocket;
    Socket_t                 xTCPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t                 xClientLength = sizeof(struct freertos_sockaddr);
    int32_t                  lBytes;
    int32_t                  lResult;
    uint32_t                 xReceiveTimeout;
    BaseType_t               xBacklog;

    uint8_t         * p_buf               = (uint8_t *)PACKET_BUFFER_ADDR;
    packet_header_t * p_rcv_packet        = (packet_header_t*)p_buf;
    uint32_t          receive_data_length = 0U;
    uint32_t          received_length     = 0U;
    
    FSP_PARAMETER_NOT_USED (pvParameters);
    
    while (FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay (100);
    }
    
    /* Allocates and initiates a socket. */
    xTCPListenSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP);
    if (FREERTOS_INVALID_SOCKET == xTCPListenSocket)
    {
        vTaskDelete(NULL);
    }
    
    xServerAddress.sin_family = FREERTOS_AF_INET;
    xServerAddress.sin_port   = FreeRTOS_htons(10000);
    xServerAddress.sin_addr   = FreeRTOS_htons(INADDR_ANY);
    
    /* Binds a socket to a local port number. */
    lResult = FreeRTOS_bind(xTCPListenSocket, &xServerAddress, sizeof(xServerAddress));
    if (lResult < 0)
    {
        vTaskDelete(NULL);
    }
    
    xReceiveTimeout = portMAX_DELAY;
    FreeRTOS_setsockopt(xTCPListenSocket , 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeout, sizeof(xReceiveTimeout));
    
    xBacklog = 10;
    FreeRTOS_listen(xTCPListenSocket, xBacklog);
    
    /* Transmission / reception processing. */
    while (1)
    {
        /* Waiting for connection request. */
        xTCPSocket = FreeRTOS_accept(xTCPListenSocket, &xClientAddress , &xClientLength);
        
        if (FREERTOS_INVALID_SOCKET == xTCPSocket)
        {
            FreeRTOS_closesocket(xTCPListenSocket);
            vTaskDelete(NULL);
        }
        else
        {
            xReceiveTimeout = portMAX_DELAY;
            FreeRTOS_setsockopt(xTCPSocket , 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeout, sizeof(xReceiveTimeout));
            
            do
            {
                /* Receive packets. */
                lBytes = FreeRTOS_recv(xTCPSocket, p_buf + received_length, (size_t)PACKET_RCV_MAX_SIZE, 0);
                
                /* Check the received data size. */
                if (lBytes > 0)
                {
                    /* First data reception over TCP communication. */
                    if (0U == received_length)
                    {
                        uint32_t rcv_payload_len = p_rcv_packet->payload_len;
                        
                        /* Get the total data size to be received. */
                        receive_data_length = sizeof(packet_header_t) + rcv_payload_len;
                        
                        /* Check the size of the received data. */
                        if (PACKET_RCV_MAX_SIZE < receive_data_length)
                        {
                            create_send_packet(p_rcv_packet, CMD_CODE_NACK, NACK_CODE_FILE_SIZE_EXCEED);
                            
                            uint32_t send_len = p_rcv_packet->payload_len;
                            send_len += sizeof(packet_header_t);
                            
                            /* Send packets. */
                            FreeRTOS_send(xTCPSocket, p_buf, (size_t)send_len, 0);
                            
                            /* If the received data size exceeds the receive buffer, */
                            /* no further data reception is performed. */
                            /* Close the TCP socket and end the process. */
                            break;
                        }
                    }
                    
                    received_length += (uint32_t)lBytes;
                    
                    /* Check if all data has been received. */
                    if (receive_data_length <= received_length)
                    {
                        /* Change the status and restart the FW update task. */
                        g_update_status_tcp = UPDATING;
                        receive_data_length = 0U;
                        received_length     = 0U;
                        vTaskResume(fwupdate_thread);
                        
                        /* Waiting for the update process to complete. */
                        update_proc_status_t status;
                        do
                        {
                            status = g_update_status_tcp;
                        } while (UPDATING == status);
                        
                        /* Check the send command. */
                        if (0U < p_rcv_packet->command)
                        {
                            uint32_t send_len = p_rcv_packet->payload_len;
                            send_len += sizeof(packet_header_t);
                            
                            /* Send packets. */
                            FreeRTOS_send(xTCPSocket, p_buf, (size_t)send_len, 0);
                        }
                    }
                }
            }
            while (lBytes > 0);
            
            /* Value initialization and communication disconnection. */
            receive_data_length = 0U;
            received_length     = 0U;
            FreeRTOS_closesocket(xTCPSocket);
        }
    }
}
