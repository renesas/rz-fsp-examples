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
#include "udp_svr_thread.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "fwupdate.h"

/******************************************************************************
 * @fn       udp_svr_thread_entry
 * @brief    UDPSvr entry function.
 *
 * @param[in]  pvParameters    Task processing parameters.
 ******************************************************************************/
void udp_svr_thread_entry(void * pvParameters)
{
    Socket_t                 xUDPSocket;
    struct freertos_sockaddr xClientAddress;
    struct freertos_sockaddr xServerAddress;
    uint32_t                 xClientLength = sizeof(struct freertos_sockaddr);
    int32_t                  lBytes;
    int32_t                  lResult;
    uint32_t                 xReceiveTimeout;

    uint8_t  * p_buf = (uint8_t *)PACKET_BUFFER_ADDR;
    packet_header_t * p_rcv_packet = (packet_header_t*)p_buf;
    
    FSP_PARAMETER_NOT_USED (pvParameters);
    
    while (FreeRTOS_IsNetworkUp() == pdFALSE)
    {
        vTaskDelay (100);
    }
    
    /* Allocates and initiates a socket. */
    xUDPSocket = FreeRTOS_socket(FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP);
    if (FREERTOS_INVALID_SOCKET == xUDPSocket)
    {
        vTaskDelete(NULL);
    }
    
    xServerAddress.sin_family = FREERTOS_AF_INET;
    xServerAddress.sin_port   = FreeRTOS_htons(10001);
    xServerAddress.sin_addr   = FreeRTOS_htons(INADDR_ANY);
    
    /* Binds a socket to a local port number. */
    lResult = FreeRTOS_bind(xUDPSocket, &xServerAddress, sizeof(xServerAddress));
    if (lResult < 0)
    {
        vTaskDelete(NULL);
    }
    
    xReceiveTimeout = portMAX_DELAY;
    FreeRTOS_setsockopt(xUDPSocket , 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeout, sizeof(xReceiveTimeout));
    
    /* Transmission / reception processing. */
    while (1)
    {
        /* Receive packets. */
        lBytes = FreeRTOS_recvfrom(xUDPSocket, p_buf, (size_t)PACKET_RCV_MAX_SIZE, 0, &xClientAddress , &xClientLength);
        
        /* Check the received data size. */
        if (lBytes > 0)
        {
            /* Change the status and restart the FW update task. */
            g_update_status_udp = UPDATING;
            vTaskResume(fwupdate_thread);
            
            /* Waiting for the update process to complete. */
            update_proc_status_t status;
            do
            {
                status = g_update_status_udp;
            } while (UPDATING == status);
            
            /* Check the send command. */
            if (0U < p_rcv_packet->command)
            {
                uint32_t send_len = p_rcv_packet->payload_len;
                send_len += sizeof(packet_header_t);
                
                /* Send packets. */
                FreeRTOS_sendto(xUDPSocket, p_buf, (size_t)send_len, 0, &xClientAddress , xClientLength);
            }
        }
        else
        {
            vTaskDelete(NULL);
        }
    }
}
