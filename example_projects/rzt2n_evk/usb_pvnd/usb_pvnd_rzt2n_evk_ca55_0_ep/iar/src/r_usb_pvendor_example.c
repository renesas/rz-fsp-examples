/***********************************************************************************************************************
 * File Name    : r_usb_pvendor_example.c
 * Description  : Contains USB Peripheral Vendor example processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include <r_usb_basic.h>

/*
 * bsp_pin_cfg.h and r_usb_bitdefine.h can define the same USB bit macros.
 * Undefine the BSP-side names before including the USB register definitions.
 */
#ifdef USB_EXICEN
 #undef USB_EXICEN
#endif

#ifdef USB_VBUSEN
 #undef USB_VBUSEN
#endif

#include "r_usb_bitdefine.h"
#include "r_usb_typedef.h"

#define USB_APL_FS

/* #define USB_APL_HS */

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define BUF_SIZE         (512U)
#define INT_TR_SIZE      (64U)
#define BUF_BULK         (0U)
#define BUF_INT          (1U)
#define START_PIPE       USB_PIPE1
#define END_PIPE         (USB_PIPE9 + 1)

#ifdef USB_APL_FS
 #define USB_APL_MXPS    (64U)
#else
 #define USB_APL_MXPS    (512U)
#endif

#define USB_VALUE_128    (0x128U)
#define USB_VALUE_FF     (0xFFU)

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static void buffer_init(void);
static void buffer_check(uint16_t buf_type, uint32_t size);
static void buffer_clear(uint16_t buf_type);

void main_task(void);

/******************************************************************************
 * Exported global variables (to be accessed by other files)
 ******************************************************************************/
static uint8_t g_buf[2][BUF_SIZE] USB_BUFFER_PLACE_IN_SECTION;
static uint8_t g_request_buf[USB_VALUE_128] USB_BUFFER_PLACE_IN_SECTION;
static bool    g_bulk_receive_confirmed = false;
static bool    g_interrupt_receive_confirmed = false;

extern const usb_cfg_t g_basic0_cfg;
extern usb_instance_ctrl_t g_basic0_ctrl;

/******************************************************************************
 * Renesas Peripheral Sample Code functions
 ******************************************************************************/
#if (BSP_CFG_RTOS == 2)

/******************************************************************************
 * Function Name   : usb_apl_callback
 * Description     : Callback function for Application program
 * Arguments       : usb_ctrl_t *p_ctrl   : Control structure for USB API.
 *                 usb_hdl_t  cur_task  : Task Handle
 *                 uint8_t    usb_state : USB_ON(USB_STATUS_REQUEST) / USB_OFF
 * Return value    : none
 ******************************************************************************/
void usb_apl_callback(usb_event_info_t * p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state)
{
    (void) usb_state;
    (void) cur_task;

    USB_APL_SND_MSG(USB_APL_MBX, (usb_msg_t *) p_api_event);
}                                      /* End of function usb_apl_callback */

#endif /* (BSP_CFG_RTOS == 2) */

//! [USB-Minimal]

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

/* Vendor class requests. */
#define USB_SET_VENDOR_NO_DATA    (0x0000U)
#define USB_SET_VENDOR            (0x0100U)
#define USB_GET_VENDOR            (0x0200U)

/******************************************************************************
 * Function Name   : main_task
 * Description     : Run the USB Peripheral Vendor event loop.
 * Arguments       : None.
 * Return value    : None.
 ******************************************************************************/
void main_task(void)
{
#if (BSP_CFG_RTOS == 2)
    usb_event_info_t * p_mess;
#endif
    usb_status_t     event;
    usb_event_info_t event_info;
    volatile uint8_t          bulk_out_pipe = 0; /* Bulk Out Pipe        */
    volatile uint8_t          bulk_in_pipe  = 0; /* Bulk In  Pipe        */
    volatile uint8_t          int_out_pipe  = 0; /* Interrupt Out Pipe   */
    volatile uint8_t          int_in_pipe   = 0; /* Interrupt In  Pipe   */

    uint16_t   buf_type       = 0;
    uint8_t    pipe           = 0;
    uint8_t    is_zlp[2]      = {0, 0};
    uint32_t   request_length = 0;
    uint16_t   used_pipe      = 0;
    usb_pipe_t pipe_info;

    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);

    while (1)
    {
#if (BSP_CFG_RTOS == 2)
        USB_APL_RCV_MSG(USB_APL_MBX, (usb_msg_t **) &p_mess);
        event_info = *p_mess;
        event      = event_info.event;
#else                                  /* (BSP_CFG_RTOS == 2) */
        g_usb_on_usb.eventGet(&event_info, &event);
#endif  /* (BSP_CFG_RTOS == 2) */
        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            {
                uint8_t bulk_out_pipe_snapshot;
                uint8_t bulk_in_pipe_snapshot;
                uint8_t int_out_pipe_snapshot;
                uint8_t int_in_pipe_snapshot;

                buffer_init();

                is_zlp[0] = 0U;
                is_zlp[1] = 0U;
                g_bulk_receive_confirmed = false;
                g_interrupt_receive_confirmed = false;

                APP_PRINT("\r\n[USB PVND] Device configured by the USB host.\r\n");

                g_usb_on_usb.usedPipesGet(&g_basic0_ctrl, &used_pipe, USB_CLASS_PVND);
                for (pipe = START_PIPE; pipe < END_PIPE; pipe++)
                {
                    if ((used_pipe & (1 << pipe)) != 0)
                    {
                        g_usb_on_usb.pipeInfoGet(&g_basic0_ctrl, &pipe_info, pipe);
                        if (USB_EP_DIR_IN != (pipe_info.endpoint & USB_EP_DIR_IN))
                        {
                            /* OUT transfer. */
                            if (USB_TRANSFER_TYPE_BULK == pipe_info.transfer_type)
                            {
                                buf_type      = BUF_BULK;
                                bulk_out_pipe = pipe;
                            }
                            else
                            {
                                buf_type     = BUF_INT;
                                int_out_pipe = pipe;
                            }
                        }
                        else
                        {
                            /* IN transfer. */
                            if (USB_TRANSFER_TYPE_BULK == pipe_info.transfer_type)
                            {
                                buf_type     = BUF_BULK;
                                bulk_in_pipe = pipe;
                            }
                            else
                            {
                                buf_type    = BUF_INT;
                                int_in_pipe = pipe;
                            }
                        }
                    }
                }

                bulk_out_pipe_snapshot = bulk_out_pipe;
                bulk_in_pipe_snapshot  = bulk_in_pipe;
                int_out_pipe_snapshot  = int_out_pipe;
                int_in_pipe_snapshot   = int_in_pipe;

                APP_PRINT("[USB PVND] Bulk OUT=%u, Bulk IN=%u, Interrupt OUT=%u, Interrupt IN=%u.\r\n",
                          (unsigned int) bulk_out_pipe_snapshot,
                          (unsigned int) bulk_in_pipe_snapshot,
                          (unsigned int) int_out_pipe_snapshot,
                          (unsigned int) int_in_pipe_snapshot);

                break;
            }

            case USB_STATUS_READ_COMPLETE:
            {
                if (FSP_ERR_USB_FAILED != event_info.status)
                {
                    if (bulk_out_pipe == event_info.pipe)
                    {
                        buf_type = BUF_BULK;
                        pipe     = bulk_in_pipe;
                        is_zlp[buf_type] = 1U;
                    }
                    else if (int_out_pipe == event_info.pipe)
                    {
                        buf_type = BUF_INT;
                        pipe     = int_in_pipe;
                        is_zlp[buf_type] = 1U;
                    }
                    else
                    {
                        APP_PRINT("[ERROR] USB PVND read completed on an unexpected pipe: %u.\r\n",
                                  (unsigned int) event_info.pipe);

                        while (1)
                        {
                            __NOP();
                            __asm volatile ("" ::: "memory");
                        }
                    }

                    /*
                     * buffer_check() returns only when the received pattern is valid.
                     * Report each successful transfer type once to avoid flooding the console.
                     */
                    buffer_check(buf_type, event_info.data_size);

                    if ((BUF_BULK == buf_type) && (false == g_bulk_receive_confirmed))
                    {
                        APP_PRINT("[PASS] USB PVND bulk data received and verified: %lu bytes.\r\n",
                                  (unsigned long) event_info.data_size);
                        g_bulk_receive_confirmed = true;
                    }
                    else if ((BUF_INT == buf_type) && (false == g_interrupt_receive_confirmed))
                    {
                        APP_PRINT("[PASS] USB PVND interrupt data received and verified: %lu bytes.\r\n",
                                  (unsigned long) event_info.data_size);
                        g_interrupt_receive_confirmed = true;
                    }
                    else
                    {
                        /* The transfer type has already been reported. */
                    }

                    g_usb_on_usb.pipeWrite(&g_basic0_ctrl,
                                           &g_buf[buf_type][0],
                                           event_info.data_size,
                                           pipe);
                }
                else
                {
                    APP_PRINT("[ERROR] USB PVND read transfer failed.\r\n");
                }

                break;
            }

            case USB_STATUS_WRITE_COMPLETE:
            {
                if (bulk_in_pipe == event_info.pipe)
                {
                    buf_type = BUF_BULK;
                    if (1U == is_zlp[buf_type])
                    {
                        pipe = bulk_out_pipe;
                    }
                }
                else if (int_in_pipe == event_info.pipe)
                {
                    buf_type = BUF_INT;
                    if (1U == is_zlp[buf_type])
                    {
                        pipe = int_out_pipe;
                    }
                }
                else
                {
                    /* No action required. */
                }

                if (1U == is_zlp[buf_type])
                {
                    is_zlp[buf_type] = 0U;

                    buffer_clear(buf_type);
                    if (BUF_BULK == buf_type)
                    {
                        g_usb_on_usb.pipeRead(&g_basic0_ctrl, &g_buf[buf_type][0], BUF_SIZE, pipe);
                    }
                    else
                    {
                        g_usb_on_usb.pipeRead(&g_basic0_ctrl, &g_buf[buf_type][0], INT_TR_SIZE, pipe);
                    }

                }
                else
                {
                    is_zlp[buf_type] = 1U;
                    if (BUF_BULK == buf_type)
                    {
                        g_usb_on_usb.pipeWrite(&g_basic0_ctrl, 0, 0, event_info.pipe); /* Send ZLP */
                    }
                    else
                    {
                        g_usb_on_usb.pipeWrite(&g_basic0_ctrl, 0, 0, event_info.pipe); /* Send ZLP */
                    }
                }

                break;
            }

            case USB_STATUS_REQUEST:
            {
                if (USB_SET_VENDOR_NO_DATA == (event_info.setup.request_type & USB_BREQUEST))
                {
                    g_usb_on_usb.periControlStatusSet(&g_basic0_ctrl, USB_SETUP_STATUS_ACK);
                }
                else if (USB_SET_VENDOR == (event_info.setup.request_type & USB_BREQUEST))
                {
                    request_length = event_info.setup.request_length;
                    g_usb_on_usb.periControlDataGet(&g_basic0_ctrl, &g_request_buf[0], request_length);
                }
                else if (USB_GET_VENDOR == (event_info.setup.request_type & USB_BREQUEST))
                {
                    g_usb_on_usb.periControlDataSet(&g_basic0_ctrl, &g_request_buf[0], request_length);
                }
                else
                {
                    /* No action required. */
                }

                break;
            }

            case USB_STATUS_REQUEST_COMPLETE:
            {
                if (USB_GET_VENDOR == (event_info.setup.request_type & USB_BREQUEST))
                {
                    g_usb_on_usb.pipeRead(&g_basic0_ctrl, &g_buf[BUF_BULK][0], BUF_SIZE, bulk_out_pipe);
                    g_usb_on_usb.pipeRead(&g_basic0_ctrl, &g_buf[BUF_INT][0], INT_TR_SIZE, int_out_pipe);
                }

                break;
            }

            case USB_STATUS_DETACH:
            {
                APP_PRINT("\r\n[USB PVND] USB host detached.\r\n");
                g_bulk_receive_confirmed = false;
                g_interrupt_receive_confirmed = false;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}                                      /* End of function main_task */

/******************************************************************************
 * Function Name   : buffer_init
 * Description     : Initialize the transfer buffers.
 * Arguments       : none
 * Return value    : none
 ******************************************************************************/
static void buffer_init(void)
{
    uint16_t i;
    uint16_t j;

    for (j = 0U; j < 2U; j++)
    {
        for (i = 0U; i < BUF_SIZE; i++)
        {
            g_buf[j][i] = (uint8_t) i;
        }
    }
}

/******************************************************************************
 * Function Name   : buffer_check
 * Description     : Verify the received transfer buffer.
 * Arguments       : buf_type : buffer number
 * Return value    : none
 ******************************************************************************/
static void buffer_check(uint16_t buf_type, uint32_t size)
{
    uint16_t i;

    for (i = 0U; i < (uint16_t) size; i++)
    {
        if ((uint8_t) (i & USB_VALUE_FF) != g_buf[buf_type][i])
        {
            APP_PRINT("[ERROR] USB PVND data verification failed at byte %u.\r\n",
                      (unsigned int) i);

            while (1)
            {
                __NOP();
                __asm volatile ("" ::: "memory");
            }
        }
    }
}

/******************************************************************************
 * Function Name   : buffer_clear
 * Description     : Clear the selected transfer buffer.
 * Arguments       : buf_type : buffer number
 * Return value    : none
 ******************************************************************************/
static void buffer_clear(uint16_t buf_type)
{
    uint16_t i;

    for (i = 0U; i < BUF_SIZE; i++)
    {
        g_buf[buf_type][i] = 0;
    }
}

/******************************************************************************
 * End of function usb_mcu_init
 ******************************************************************************/

#if (BSP_CFG_RTOS == 2)

/******************************************************************************
 * Function Name   : usb_apl_rec_msg
 * Description     : Receive a message to the specified id (mailbox).
 * Argument        : uint8_t id        : ID number (mailbox).
 *               : usb_msg_t** mess  : Message pointer
 *               : usb_tm_t tm       : Timeout Value
 * Return          : uint16_t          : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_apl_rec_msg(uint8_t id, usb_msg_t ** mess, usb_tm_t tm)
{
    BaseType_t    err;
    QueueHandle_t handle;
    usb_er_t      result;

    (void) tm;

    if (NULL == mess)
    {
        return USB_APL_ERROR;
    }

    handle = (*(g_apl_mbx_table[id]));
    *mess  = NULL;

    /* Receive a message from the queue specified by the mailbox table. */
    err = xQueueReceive(handle, (void *) mess, (portMAX_DELAY));
    if ((pdTRUE == err) && (NULL != (*mess)))
    {
        result = USB_APL_OK;
    }
    else
    {
        result = USB_APL_ERROR;
    }

    return result;
}

/******************************************************************************
 * End of function usb_apl_rec_msg
 ******************************************************************************/

/******************************************************************************
 * Function Name   : usb_apl_snd_msg
 * Description     : Send a message to the specified id (mailbox).
 * Argument        : uint8_t id        : ID number (mailbox).
 *               : usb_msg_t* mess   : Message pointer
 * Return          : usb_er_t          : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_apl_snd_msg(uint8_t id, usb_msg_t * mess)
{
    BaseType_t    err;
    QueueHandle_t handle;
    usb_er_t      result;

    if (NULL == mess)
    {
        return USB_APL_ERROR;
    }

    handle = (*(g_apl_mbx_table[id]));

    /* Send a message to the queue specified by the mailbox table. */
    err = xQueueSend(handle, (const void *) &mess, (TickType_t) (0));
    if (pdTRUE == err)
    {
        result = USB_APL_OK;
    }
    else
    {
        result = USB_APL_ERROR;
    }

    return result;
}

/******************************************************************************
 * End of function usb_apl_snd_msg
 ******************************************************************************/
#endif                                 /* #if (BSP_CFG_RTOS == 2) */

//! [USB-Minimal]

/******************************************************************************
 * End  Of File
 ******************************************************************************/
