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
#include "net_thread.h"
#include "FreeRTOS_IP.h"

/******************************************************************************
 * Exported global variables and functions (to be accessed by other files)
 ******************************************************************************/
extern TaskHandle_t net_thread;

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static uint8_t s_ucIPAddress[4]        = {192, 168, 10, 100};
static uint8_t s_ucNetMask[4]          = {255, 255, 255, 0};
static uint8_t s_ucGatewayAddress[4]   = {192, 168, 10, 3};
static uint8_t s_ucDNSServerAddress[4] = {10, 60, 1, 2};

/******************************************************************************
 * @fn       net_thread_entry
 * @brief    Net thread entry function.
 *
 * @param[in]  pvParameters         Task processing parameters.
 ******************************************************************************/
void net_thread_entry(void * pvParameters)
{
    BaseType_t status = pdFALSE;
    
    FSP_PARAMETER_NOT_USED (pvParameters);
    
    /* FreeRTOS IP Initialization: This init initializes the IP stack  */
    status = FreeRTOS_IPInit(s_ucIPAddress, s_ucNetMask, s_ucGatewayAddress, s_ucDNSServerAddress, gp_freertos_ether->p_cfg->p_mac_address);
    if (pdFALSE == status)
    {
        vTaskDelete(NULL);
    }
    
    /* Suspend this task. */
    vTaskSuspend(net_thread);
    while (1);
}
