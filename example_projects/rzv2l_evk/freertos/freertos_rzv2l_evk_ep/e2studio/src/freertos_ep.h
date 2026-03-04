/***********************************************************************************************************************
 * File Name    : freertos_ep.h
 * Description  : Contains macros, data structures and functions declaration used common to the EP
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef FREERTOS_EP_H_
#define FREERTOS_EP_H_

#define EP_INFO     "\r\nMessages are shared between Sender_Task, ISR and Receiver_Task for the first few seconds.\n"\
                    "Later, Sender and Receiver Tasks will be suspended timer will be stopped. Semaphore is acquired\n"\
                    "and released between Semaphore Task and ISR for the next few seconds and Semaphore Task is"\
                    "suspended.\nTo restart the application, power cycle the board.\n"

#define WAIT_TIME           (500u)  /* Wait time for task on Semaphore and Message Queue. */
#define TASK_SUSPEND_COUNT  (10u)   /* ISR iterations. */

/* Message structure. */
typedef struct {
    uint8_t    data;
    char *     p_task_name;
}st_msg_t;
/* Task handles. */
extern TaskHandle_t receiver_task;
extern TaskHandle_t semaphore_task;

#endif /* FREERTOS_EP_H_ */
