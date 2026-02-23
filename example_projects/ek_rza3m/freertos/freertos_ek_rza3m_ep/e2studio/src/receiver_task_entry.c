/***********************************************************************************************************************
 * File Name    : receiver_task_entry.c
 * Description  : Contains data structures and functions used in receiver_task_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "receiver_task.h"
#include "common_utils.h"
#include "freertos_ep.h"
#include "gtm_timer_initialize.h"

/***********************************************************************************************************************
 * @fn           receiver_task_entry
 * @brief        This function receives messages from queue and print data and sender name.
 * @param[in]    pvParameters  Pointer to data.
 * @retval       None.
 **********************************************************************************************************************/
void receiver_task_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Message variable to receive message */
    msg_t message_from_task = {RESET_VALUE, (char *)NULL};

    while (true)
    {
        /* Receive messages until queue is empty and print same */
        if (pdPASS == xQueueReceive(g_queue, &message_from_task, portMAX_DELAY))
        {
            APP_PRINT ("\n\rReceiver Task : Message received Successfully");
            APP_PRINT ("\n\r    Data   : %d  \r\n    Sender : %s\r\n", message_from_task.data,
                       message_from_task.task_name);
        }
    }
}
/***********************************************************************************************************************
 End of function receiver_task_entry
 **********************************************************************************************************************/
