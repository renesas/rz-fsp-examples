/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "main_thread.h"
/* Main Thread entry function */
/* pvParameters contains TaskHandle_t */
void main_thread_entry(void *pvParameters)
{
    FSP_PARAMETER_NOT_USED (pvParameters);

    /** Launch the application example of Free RTOS+TCP. */
    extern void tlsapp_mbedtls_freertostcp_user_main( void );
    (void) tlsapp_mbedtls_freertostcp_user_main();

    /* TODO: add your own code here */
    while (1)
    {
        vTaskDelay (1000);
    }
}
