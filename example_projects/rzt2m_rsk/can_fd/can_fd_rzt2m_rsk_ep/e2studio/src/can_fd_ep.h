/***********************************************************************************************************************
 * File Name    : can_fd_ep.h
 * Description  : Contains declarations of data structures and functions used in can_fd_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CAN_FD_EP_H_
#define CAN_FD_EP_H_

#include "common_utils.h"

#define CAN_MAILBOX_NUMBER_0            (0U)               /* mail box number */
#define CAN_CLASSIC_FRAME_DATA_BYTES    (8U)               /* Data Length code for classic frame */
#define CAN_FD_DATA_LENGTH_CODE         (64)               /* Data Length code for FD frame */
#define CAN_ID                          (0x20)             /* ID to be updated in transmit frame */
/* Acceptance filter array parameters */
#define CANFD_FILTER_ID                 (0x00001000)
#define MASK_ID                         (0x1FFFF000)
#define MASK_ID_MODE                    (1)
#undef  BUFFER_SIZE_UP
#define BUFFER_SIZE_UP                  (2048)
#define ZERO                            (0U)               /* Array Index value */
#define NULL_CHAR                       ('\0')             /* MACRO for null character */
#define WAIT_TIME                       (10000U)             /* Wait time value */
/** Timeout value */
extern volatile uint32_t g_time_out;

/* Flags to be set in Callback function */
extern volatile bool g_b_canfd_err_status;

/***********************************************************************************************************************
 * @fn          canfd_operation
 * @brief       This function is to transmit data after receiving user input and it performs subsequent operations
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
void canfd_operation (void);

/***********************************************************************************************************************
 * @fn          can_read_operation
 * @brief       This function is to read Channel status info and read data.
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
void can_read_operation (void);

/***********************************************************************************************************************
 * @fn          canfd_deinit
 * @brief       This function is to de-initializes the CANFD module
 * @param[in]   None
 * @return      None
 **********************************************************************************************************************/
void canfd_deinit (void);

/***********************************************************************************************************************
 *  Function Name: canfd_entry
 *  Description  : This function is used to start the canfd example operation.
 *  Arguments    : None
 *  Return Value : None
 **********************************************************************************************************************/
void canfd_entry (void);

#endif /* CAN_FD_EP_H_ */
