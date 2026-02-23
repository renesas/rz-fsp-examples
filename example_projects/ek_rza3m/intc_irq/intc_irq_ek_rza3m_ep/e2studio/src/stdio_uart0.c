/***********************************************************************************************************************
 * File Name    : stdio_uart0.c
 * Description  : Contains data structures and functions used in stdio_uart0.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/

#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "bsp_api.h"
#include "hal_data.h"

/* #include "r_os_abstraction_api.h" */
#if (BSP_CFG_RTOS == 2)
#include "event_groups.h"
#include "semphr.h"
#endif

/******************************************************************************
 Macro Definitions
 ******************************************************************************/

/******************************************************************************
 Private global variables and functions
 ******************************************************************************/
int  stdio_open (void);
void stdio_close (void);
int  stdio_read (uint8_t * pbyBuffer, uint32_t uiCount);
int  stdio_write (const uint8_t * pbyBuffer, uint32_t uiCount);

void app_read (unsigned char * p_data);
int app_check_data ();

/******************************************************************************
 Imported global variables and functions (from other files)
 ******************************************************************************/
static uint32_t s_stdio_initialized = 0;
#if (BSP_CFG_RTOS == 2)
static EventGroupHandle_t  printf_event = NULL;
static SemaphoreHandle_t  printf_semaphore = NULL;
#else
static volatile int s_wait_tx = 0;
#endif
static volatile uint32_t s_scan_write = 0;
static          uint32_t s_scan_read  = 0;
static          uint8_t  s_scan_buffer[1024];

char g_buff[1024];   // Stream buff

/**********************************************************************************************************************
 Private global variables and functions
 **********************************************************************************************************************/
/* Dummy driver configuration */

int stdio_open(void)
{
    int ret = 0;
    if (0 == s_stdio_initialized)
    {
#if (BSP_CFG_RTOS == 2)
        printf_event = xEventGroupCreate();
        printf_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(printf_semaphore);
#endif
        s_scan_write = 0;
        s_scan_read  = 0;
        memset(s_scan_buffer, 0, sizeof(s_scan_buffer));
        s_stdio_initialized = 1;
    }
    ret = g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);
    g_uart0.p_api->read(g_uart0.p_ctrl, &s_scan_buffer[s_scan_write], 0);
    return ret;
}

void stdio_close(void)
{
    g_uart0.p_api->close(g_uart0.p_ctrl);
}

int stdio_read(uint8_t *pbyBuffer, uint32_t uiCount)
{
#if 1 /* if you use only getchar for standard input, change 1 -> 0 */
    uint32_t ret = 0;
    uint8_t  current_data;
    if (0 == s_stdio_initialized)
    {
        stdio_open();
    }

    while (ret < uiCount)
    {
        while (s_scan_write == s_scan_read)
        {
#if (BSP_CFG_RTOS == 2)
            vTaskDelay(1);
#else
#endif
        }
        current_data = s_scan_buffer[s_scan_read];
        *pbyBuffer   = current_data;
        s_scan_read++;
        pbyBuffer++;
        ret++;
        if (s_scan_read >= (sizeof(s_scan_buffer)))
        {
            s_scan_read = 0;
        }
        if ('\n' == current_data)
        {
            break;
        }
        if ('\r' == current_data)
        {
            if ('\n' != s_scan_buffer[s_scan_read])
            {
                break;
            }
        }
    }

    /* Cast from uint32_t to int */
    return (int)ret;
#else
    int ret = 0;

    if (s_stdio_initialized == 0)
    {
        stdio_open();
    }
    while(s_scan_write == s_scan_read)
    {
#if (BSP_CFG_RTOS == 2)
        vTaskDelay(1);
#else
#endif
    }

    if( s_scan_write < s_scan_read )
    {
        if( s_scan_read + uiCount < sizeof(s_scan_buffer) )
        {
            ret = uiCount;
        }
        else
        {
            ret = sizeof(s_scan_buffer) - s_scan_read;
        }
    }
    else
    {
        if( s_scan_read + uiCount < s_scan_write )
        {
            ret = uiCount;
        }
        else
        {
            ret = s_scan_write - s_scan_read;
        }
    }
    memcpy(pbyBuffer,&s_scan_buffer[s_scan_read],ret);
    s_scan_read += ret;
    if( s_scan_read > sizeof(s_scan_buffer) )
    {
        s_scan_read = 0;
    }
    return ret;
#endif
}

int stdio_write(const uint8_t * pbyBuffer, uint32_t uiCount)
{
    int ret;

    if (0 == s_stdio_initialized)
    {
        stdio_open();
    }
#if (BSP_CFG_RTOS == 2)
    xSemaphoreTake(printf_semaphore,portMAX_DELAY);
#else
    s_wait_tx = 0;
#endif
    ret = g_uart0.p_api->write(g_uart0.p_ctrl, pbyBuffer, uiCount);
#if (BSP_CFG_RTOS == 2)
    xEventGroupWaitBits(printf_event,1,pdTRUE,pdTRUE,portMAX_DELAY);
    xSemaphoreGive(printf_semaphore);
#else
    while(s_wait_tx == 0);
#endif
    return ret;
}

void uart0_cb_stdio (uart_callback_args_t * p_args)
{
    if (0 == s_stdio_initialized)
    {
        stdio_open();
    }
    if (NULL != p_args)
    {
        if (p_args->event & UART_EVENT_TX_COMPLETE)
        {
#if (BSP_CFG_RTOS == 2)
            BaseType_t xHigherPriorityTaskWoken, xResult;

            /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
            xHigherPriorityTaskWoken = pdFALSE;

            /* Set bit 0 and bit 4 in xEventGroup */
            xResult = xEventGroupSetBitsFromISR(
                                  printf_event,   /* The event group being updated. */
                                  1, /* The bits being set. */
                                  &xHigherPriorityTaskWoken );

            /* Was the message posted successfully? */
            if( xResult != pdFAIL )
            {
                /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                switch should be requested.  The macro used is port specific and will
                be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
                the documentation page for the port being used. */
                portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
            }
#else
            s_wait_tx = 1;
#endif
        }
        if (p_args->event & UART_EVENT_RX_CHAR)
        {
            /* Cast from uint32_t to uint8_t */
            s_scan_buffer[s_scan_write] = (uint8_t)p_args->data;


            if (s_scan_write < (sizeof(s_scan_buffer)))
            {
                s_scan_write++;
            }
            if (s_scan_write >= (sizeof(s_scan_buffer)))
            {
                s_scan_write = 0;
            }
        }
    }
}

void app_read(unsigned char* p_data)
{
    if (0 == s_stdio_initialized)
    {
        stdio_open();
    }
    scanf("%1023s", p_data);
    sscanf(g_buff, "%1023s", p_data);
    fflush(stdin);
}

int app_check_data()
{
    if (0 == s_stdio_initialized)
    {
        stdio_open();
    }

    if (s_scan_write != s_scan_read)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/***********************************************************************************************************************
 End Of File
 **********************************************************************************************************************/
