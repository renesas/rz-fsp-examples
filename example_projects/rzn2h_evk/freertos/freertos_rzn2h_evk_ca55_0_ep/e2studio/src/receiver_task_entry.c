/***********************************************************************************************************************
 * File Name    : receiver_task_entry.c
 * Description  : Contains data structures and functions used in receiver_task_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "receiver_task.h"
#include "common_utils.h"
#include "freertos_ep.h"
#include "r_cmt_timer_initialize.h"

/***********************************************************************************************************************
 * @addtogroup freertos_ep
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: receiver_task_entry
 * Description  : Receive messages from queue and print data and sender name.
 * Argument     : pvParameters
 * Return Value : None
 **********************************************************************************************************************/
void receiver_task_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Message variable to receive message */
    st_msg_t message_from_task = {RESET_VALUE, (char *)NULL};

    while (true)
    {
        /* Receive messages until queue is empty and print same */
        if (pdPASS == xQueueReceive (g_queue, &message_from_task, portMAX_DELAY))
        {
            APP_PRINT ("\r\nReceiver Task : Message received Successfully\r\n");
            APP_PRINT ("\r\n    Data   : %d  \r\n    Sender : %s\r\n",
                        message_from_task.data,
                        message_from_task.task_name);
        }
    }
}
/***********************************************************************************************************************
 End of function receiver_task_entry
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @} (end addtogroup freertos_ep)
 **********************************************************************************************************************/
