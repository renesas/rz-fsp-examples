/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "main_thread.h"
#include "um_common.h"

/* Main Thread entry function */
/* pvParameters contains TaskHandle_t */
void main_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /** Launch the initialization example of lwIP port. */
    extern usr_err_t lwip_port_user_instance_init( void );
    if (USR_SUCCESS == lwip_port_user_instance_init())
    {
        /** Launch the application example of lwIP port. */
        extern void tlsapp_mbedtls_lwip_user_main( void );
        (void) tlsapp_mbedtls_lwip_user_main();
    }

    /** TODO: add your own code here */
    while (1)
    {
        vTaskSuspend(NULL);
    }
}
