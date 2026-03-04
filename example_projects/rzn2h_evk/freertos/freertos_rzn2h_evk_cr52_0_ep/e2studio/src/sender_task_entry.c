/***********************************************************************************************************************
 * File Name    : sender_task_entry.c
 * Description  : Contains data structures and functions used in sender_task_entry.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "sender_task.h"
#include "common_utils.h"
#include "freertos_ep.h"
#include "r_cmt_timer_initialize.h"

/***********************************************************************************************************************
 * @addtogroup freertos_ep
 * @{
 **********************************************************************************************************************/
/* Flag to check Message Queue task completion */
static bool s_suspend_msq_queue_task = false;

/***********************************************************************************************************************
 * Function Name: sender_task_entry
 * Description  : Initialize R_CMT timer and periodically send messages to the queue; on suspend request, stop/close
 *                the timer, suspend the receiver task, resume the semaphore task, then suspend self.
 * Argument     : pvParameters
 * Return Value : None
 **********************************************************************************************************************/
void sender_task_entry(void * pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /* Variable to store task state */
    eTaskState task_state = (eTaskState) RESET_VALUE;

    fsp_pack_version_t version = {RESET_VALUE};

    /* Initialize the message to send to Message Queue */
    st_msg_t s_message_to_task = {100 , "Sender_Task"};

    /* Get the FSP version */
    R_FSP_VersionGet (&version);

    /* Example Project information printed on the RTT */
    APP_PRINT (BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);
    APP_PRINT (EP_INFO);

    /* The rate at which the task waits on the Message availability */
    TickType_t Queue_wait_ticks = pdMS_TO_TICKS (WAIT_TIME);

    /* Initialize the variable used by the call to vTaskDelayUntil() */
    TickType_t last_execution_time = xTaskGetTickCount ();

    /* Check the task state, if in running or ready state only,
     * suspend Semaphore task. Semaphore task will be resumed after
     * Message Queue task has completed execution */
    task_state = eTaskGetState (semaphore_task);
    if ((eBlocked == task_state) || (eRunning == task_state) || (eReady == task_state))
    {
        vTaskSuspend (semaphore_task);
    }

    APP_PRINT ("\r\n Sender_Task : Starting g_periodic_timer_msgq timer");

    /* Start R_CMT timer to start sending message to Message Queue periodically at 1sec from R_CMT callback */
    fsp_err_t fsp_err = r_cmt_timer_init (&g_periodic_timer_msgq_ctrl, &g_periodic_timer_msgq_cfg);

    /* Handle error */
    if (FSP_SUCCESS != fsp_err)
    {
        APP_ERR_TRAP(fsp_err);
    }

    while (true)
    {
        /* Check if flag is set, to suspend Message Queue task. If not set,
         * continue sending messages to Message Queue at 500ms periodically */
        if (true != s_suspend_msq_queue_task)
        {
            /* Send message to back of Message Queue */
            if (pdPASS == xQueueSendToBack (g_queue , &s_message_to_task , Queue_wait_ticks))
            {
                APP_PRINT ("\r\n Sender_Task : Message posted on Queue successfully");
                APP_PRINT ("\r\n Sender_Task : Going on delay for 500ms");

                /* Places the task into the Blocked state. The task
                 * will execute every 500ms once */
                vTaskDelayUntil (&last_execution_time , WAIT_TIME);

                APP_PRINT ("\r\n Sender_Task : After delay of 500ms\r\n");
            }
        }
        else
        {
            /* Flush the message from the Queue before suspending the task */
            if (pdPASS == xQueueReset (g_queue))
            {
                APP_PRINT ("\r\nSender_Task : Message Queue reset \r\n");

                /* Suspend Message Queue tasks and resume Semaphore task for Execution */
                fsp_err = R_CMT_Stop (&g_periodic_timer_msgq_ctrl);

                /* Handle error */
                if (FSP_SUCCESS != fsp_err)
                {
                    if (R_CMT_Close(&g_periodic_timer_msgq_ctrl) != FSP_SUCCESS)
                    {
                        /* Print out in case of error */
                        APP_ERR_PRINT ("\r\nSender_Task : R_CMT Timer Close API failed\r\n");
                    }

                    /* Print out in case of error */
                    APP_ERR_PRINT ("\r\nSender_Task : R_CMT Timer stop API failed\r\n");
                    APP_ERR_TRAP (fsp_err);
                }
                APP_PRINT ("\r\nSender_Task : R_CMT timer stopped\r\n");

                fsp_err = R_CMT_Close (&g_periodic_timer_msgq_ctrl);
                if (FSP_SUCCESS != fsp_err)
                {
                    /* Print out in case of error */
                    APP_ERR_PRINT ("\r\nSender_Task : R_CMT Timer Close API failed\r\n");
                    APP_ERR_TRAP (fsp_err);
                }

                vTaskSuspend (receiver_task);
                APP_PRINT ("\r\nSender_Task : Receiver Task Suspended\r\n");

                /* Before suspending current task resume the semaphore task */
                vTaskResume (semaphore_task);
                APP_PRINT ("\r\nSender_Task : Semaphore Task Resumed\r\n");
                APP_PRINT ("\r\nSender_Task : Suspending Sender Task\r\n");

                /* Suspends calling task */
                vTaskSuspend (RESET_VALUE);
            }
        }
    }
}
/***********************************************************************************************************************
 End of function sender_task_entry
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: periodic_timer_msgq_cb
 * Description  : User defined R_CMT irq callback for Message Queue.
 * Argument     : p_args
 * Return Value : None
 **********************************************************************************************************************/
void periodic_timer_msgq_cb(timer_callback_args_t *p_args)
{
    FSP_PARAMETER_NOT_USED (p_args);

    /* Counter to track task suspend count */
    static uint8_t s_msg_queue_counter = RESET_VALUE;

    /* Variable is set to true if priority of unblocked task is higher
     * than the task that was in running state when interrupt occurred */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Initialize the message to send to Message Queue */
    static st_msg_t s_message_to_task = {200 , "R_CMT Callback"};

    /* If ISR is executed for 10sec then set the flag to suspend task */
    if (TASK_SUSPEND_COUNT == s_msg_queue_counter)
    {
        /* Set flag to suspend Message Queue tasks */
        s_suspend_msq_queue_task = true;
    }
    else
    {
        /* Send message to Message Queue */
        if (pdPASS ==  xQueueSendToBackFromISR (g_queue , &s_message_to_task , &xHigherPriorityTaskWoken))
        {
            /* Increment counter on successfully sending message */
            s_msg_queue_counter++;
        }
    }

    /* A context switch will be performed if xHigherPriorityTaskWoken is equal to pdTRUE */
    portYIELD_FROM_ISR (xHigherPriorityTaskWoken);
}
/***********************************************************************************************************************
 End of function periodic_timer_msgq_cb
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @} (end addtogroup freertos_ep)
 **********************************************************************************************************************/
