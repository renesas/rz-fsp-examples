/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "FreeRTOS.h"
#include "FreeRTOS_Sockets.h"

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**
 * Endpoint's socket context structure of FreeRTOS-Plus-TCP for SSL/TLS communication.
 */
 typedef struct st_tlsapp_mbedtls_sock_ctx_info
 {
     Socket_t    sock_hdl;   /** Socket handle */
     BaseType_t  send_flag;  /** Send flag */
     BaseType_t  recv_flg;   /** Receive flag */
 } tlsapp_mbedtls_sock_ctx_info_t;
