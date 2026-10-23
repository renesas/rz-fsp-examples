/***********************************************************************************************************************
 * File Name    : stdio_uart0.c
 * Description  : Contains data structures and functions used in stdio_uart0.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 Includes   <System Includes> , "Project Includes"
 ******************************************************************************/

#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include "bsp_api.h"
#include "hal_data.h"
/*  #include "r_os_abstraction_api.h"  */
#if (BSP_CFG_RTOS == 2)
#include "event_groups.h"
#include "semphr.h"
#endif

/******************************************************************************
 Macro Definitions
 ******************************************************************************/
#define BUF_SIZE        (16U)
 
/******************************************************************************
Private global variables and functions
 ******************************************************************************/
int stdio_open (void);
void stdio_close  (void);
int stdio_read (uint8_t * pbyBuffer, uint32_t uiCount);
int stdio_write (const uint8_t * pbyBuffer, uint32_t uiCount);

void app_read(unsigned char * p_data);
int app_check_data ();
/******************************************************************************
 Imported global variables and functions (from other files)
 ******************************************************************************/
static uint32_t stdio_initialized = 0;
#if (BSP_CFG_RTOS == 2)
static EventGroupHandle_t  printf_event = NULL;
static SemaphoreHandle_t  printf_semaphore = NULL;
#else
static volatile int wait_tx = 0;
#endif
static volatile uint32_t scan_write = 0;
static volatile uint32_t scan_read = 0;
static uint8_t scan_buffer[1024];

char g_buff[1024];   // Stream buff
/**********************************************************************************************************************
 Private global variables and functions
 **********************************************************************************************************************/
/* Dummy driver configuration */


int stdio_open(void)
{
    int ret = 0;
    if (0 == stdio_initialized)
    {
#if (BSP_CFG_RTOS == 2)
        printf_event = xEventGroupCreate();
        printf_semaphore = xSemaphoreCreateBinary();
        xSemaphoreGive(printf_semaphore);
#endif
        scan_write = 0;
        scan_read = 0;
        memset(scan_buffer,0,sizeof(scan_buffer));
        stdio_initialized = 1;
    }
    ret = g_uart0.p_api->open(g_uart0.p_ctrl, g_uart0.p_cfg);
    g_uart0.p_api->read(g_uart0.p_ctrl, &scan_buffer[scan_write], 0);
    setvbuf(stdout, NULL, _IONBF, 0);
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
    if (0 == stdio_initialized)
    {
        stdio_open();
    }

    while (ret < uiCount)
    {
        while (scan_write == scan_read)
        {
#if (BSP_CFG_RTOS == 2)
            vTaskDelay(1);
#else
#endif
        }
        current_data = scan_buffer[scan_read];
        *pbyBuffer = current_data;
        scan_read++;
        pbyBuffer++;
        ret++;
        if (scan_read >= sizeof(scan_buffer))
        {
            scan_read = 0;
        }
        if ('\n' == current_data)
        {
            break;
        }
        if ('\r' == current_data)
        {
            if ('\n' != scan_buffer[scan_read])
            {
                break;
            }
        }
    }
    return (int)ret;
#else
    int ret = 0;

    if (stdio_initialized == 0)
    {
        stdio_open();
    }
    while(scan_write == scan_read)
    {
#if (BSP_CFG_RTOS == 2)
        vTaskDelay(1);
#else
#endif
    }

    if( scan_write < scan_read )
    {
        if( scan_read + uiCount < sizeof(scan_buffer) )
        {
            ret = uiCount;
        }
        else
        {
            ret = sizeof(scan_buffer) - scan_read;
        }
    }
    else
    {
        if( scan_read + uiCount < scan_write )
        {
            ret = uiCount;
        }
        else
        {
            ret = scan_write - scan_read;
        }
    }
    memcpy(pbyBuffer,&scan_buffer[scan_read],ret);
    scan_read += ret;
    if( scan_read > sizeof(scan_buffer) )
    {
        scan_read = 0;
    }
    return ret;
#endif
}

int stdio_write(const uint8_t * pbyBuffer, uint32_t uiCount)
{
    int ret;

    if (0 == stdio_initialized)
    {
        stdio_open();
    }
#if (BSP_CFG_RTOS == 2)
    xSemaphoreTake(printf_semaphore,portMAX_DELAY);
#else
    wait_tx = 0;
#endif
    ret = g_uart0.p_api->write(g_uart0.p_ctrl, pbyBuffer, uiCount);
#if (BSP_CFG_RTOS == 2)
    xEventGroupWaitBits(printf_event,1,pdTRUE,pdTRUE,portMAX_DELAY);
    xSemaphoreGive(printf_semaphore);
#else
    while(wait_tx == 0);
#endif
    return ret;
}

void uart0_cb_stdio(uart_callback_args_t * p_args)
{
    uint32_t next_write;

    if ((NULL == p_args) || (0 == stdio_initialized))
    {
        return;
    }

    switch (p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
        {
#if (BSP_CFG_RTOS == 2)
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            BaseType_t xResult;

            xResult = xEventGroupSetBitsFromISR(printf_event,
                                                1,
                                                &xHigherPriorityTaskWoken);

            if (pdFAIL != xResult)
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
#else
            wait_tx = 1;
#endif
            break;
        }

        case UART_EVENT_RX_CHAR:
        {
            next_write = scan_write + 1U;

            if (next_write >= sizeof(scan_buffer))
            {
                next_write = 0U;
            }

            /* Ring buffer full */
            if (next_write == scan_read)
            {
            }
            else
            {
                scan_buffer[scan_write] = (uint8_t) p_args->data;
                scan_write = next_write;
            }
            break;
        }

        case UART_EVENT_ERR_OVERFLOW:
        {
            break;
        }

        default:
        {
            break;
        }
    }
}

void app_read(unsigned char * p_data)
{
    uint32_t index = 0;
    uint8_t ch;

    memset(p_data, 0, BUF_SIZE);

    while ((scan_read != scan_write) && (index < (BUF_SIZE - 1)))
    {
        ch = scan_buffer[scan_read];

        scan_read++;
        if (scan_read >= sizeof(scan_buffer))
        {
            scan_read = 0;
        }

        /* End of line */
        if ((ch == '\r') || (ch == '\n'))
        {
            break;
        }

        p_data[index++] = ch;
    }

    p_data[index] = '\0';
}


int app_check_data(void)
{
    uint32_t pos = scan_read;

    while (pos != scan_write)
    {
        if ((scan_buffer[pos] == '\r') ||
            (scan_buffer[pos] == '\n'))
        {
            return 1;
        }

        pos++;
        if (pos >= sizeof(scan_buffer))
        {
            pos = 0;
        }
    }

    return 0;
}
/***********************************************************************************************************************
 End  Of File
 **********************************************************************************************************************/
