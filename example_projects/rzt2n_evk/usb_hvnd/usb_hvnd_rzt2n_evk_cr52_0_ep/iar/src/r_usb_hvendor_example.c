/***********************************************************************************************************************
 * File Name    : r_usb_hvendor_example.c
 * Description  : Contains USB Host Vendor example processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include <r_usb_basic.h>
#include "r_usb_typedef.h"

/*
 * bsp_pin_cfg.h and r_usb_bitdefine.h define USB_EXICEN and USB_VBUSEN for
 * different purposes. The USB driver definitions are required in this file.
 * Undefine the BSP pin aliases before including the USB bit definitions to
 * avoid macro-redefinition warnings without changing the resulting values.
 */
#ifdef USB_EXICEN
 #undef USB_EXICEN
#endif
#ifdef USB_VBUSEN
 #undef USB_VBUSEN
#endif

#include "r_usb_bitdefine.h"
#include "r_usb_basic_local.h"

#define USB_APL_HS

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define BUF_SIZE       (512U)
#define INT_TR_SIZE    (64U)
#define REQ_SIZE       (125U)

#ifdef USB_APL_FS
 #define USB_APL_MXPS  (64U)
#else
 #define USB_APL_MXPS  (512U)
#endif

#define ADDRESS1       (1U)
#define START_PIPE     (USB_PIPE1)
#define END_PIPE       (USB_PIPE9 + 1)
#define BUF_BULK       (0U)
#define BUF_INT        (1U)

#define USB_VALUE_128  (0x128U)
#define USB_VALUE_FF   (0xFFU)

/* Vendor Class Requests. */
#define USB_SET_VENDOR_NO_DATA  (0x0000U)
#define USB_SET_VENDOR          (0x0100U)
#define USB_GET_VENDOR          (0x0200U)

#define SET_VENDOR_NO_DATA  (USB_SET_VENDOR_NO_DATA | USB_HOST_TO_DEV | USB_VENDOR | USB_INTERFACE)
#define SET_VENDOR          (USB_SET_VENDOR | USB_HOST_TO_DEV | USB_VENDOR | USB_INTERFACE)
#define GET_VENDOR          (USB_GET_VENDOR | USB_DEV_TO_HOST | USB_VENDOR | USB_INTERFACE)

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static void buffer_init(void);
static void buffer_check(uint16_t buf_type, uint32_t size);
static void buffer_clear(uint16_t buf_type);

static void class_request_set_vendor(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
static void class_request_get_vendor(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
static void class_request_set_vendor_no_data(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

void main_task(void);

#if (BSP_CFG_RTOS == 2)
void usb_apl_callback(usb_event_info_t * p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state);
usb_er_t usb_apl_rec_msg(uint8_t id, usb_msg_t ** mess, usb_tm_t tm);
usb_er_t usb_apl_snd_msg(uint8_t id, usb_msg_t * mess);
#endif

/******************************************************************************
 * Exported global variables
 ******************************************************************************/
static uint8_t g_buf[2][BUF_SIZE] USB_BUFFER_PLACE_IN_SECTION;
static uint8_t g_request_buf[USB_VALUE_128] USB_BUFFER_PLACE_IN_SECTION;

/*
 * One-shot console status flags.
 * A transfer is reported as PASS only after the Host receives the echoed
 * payload from PVND and buffer_check() verifies the complete data pattern.
 */
static bool g_bulk_roundtrip_confirmed      = false;
static bool g_interrupt_roundtrip_confirmed = false;
static bool g_all_roundtrip_confirmed       = false;

uint8_t debug_info[64];
uint8_t debug_info1[64];
uint8_t debug_info2[64];
uint8_t debug_cnt = 0U;

extern const usb_cfg_t      g_basic0_cfg;
extern usb_instance_ctrl_t  g_basic0_ctrl;

#if (BSP_CFG_RTOS == 2)
/******************************************************************************
 * Function Name: usb_apl_callback
 * Description  : Callback function for the USB application program.
 * Arguments    : p_api_event -
 *                    Pointer to USB event information.
 *                cur_task -
 *                    USB task handle.
 *                usb_state -
 *                    USB state.
 * Return Value : None.
 ******************************************************************************/
void usb_apl_callback(usb_event_info_t * p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state)
{
    (void) usb_state;
    (void) cur_task;

    USB_APL_SND_MSG(USB_APL_MBX, (usb_msg_t *) p_api_event);
}

/*******************************************************************************
 * End of function usb_apl_callback
 ******************************************************************************/
#endif

/******************************************************************************
 * Function Name: main_task
 * Description  : Run the USB Host Vendor application.
 * Arguments    : None.
 * Return Value : None.
 ******************************************************************************/
void main_task(void)
{
#if (BSP_CFG_RTOS == 2)
    usb_event_info_t * p_mess;
#endif

    usb_status_t     event;
    usb_event_info_t event_info;
    uint8_t          bulk_out_pipe = 0U;
    uint8_t          bulk_in_pipe  = 0U;
    uint8_t          int_out_pipe  = 0U;
    uint8_t          int_in_pipe   = 0U;

    uint16_t   buf_type  = 0U;
    uint8_t    pipe      = 0U;
    uint8_t    is_zlp[2] = {0U, 0U};
    uint16_t   used_pipe = 0U;
    usb_pipe_t pipe_info;

    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);

    while (1)
    {
#if (BSP_CFG_RTOS == 2)
        USB_APL_RCV_MSG(USB_APL_MBX, (usb_msg_t **) &p_mess);
        event_info = *p_mess;
        event      = event_info.event;
#else
        g_usb_on_usb.eventGet(&event_info, &event);
#endif

        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            {
                buffer_init();

                is_zlp[0] = 0U;
                is_zlp[1] = 0U;

                g_bulk_roundtrip_confirmed      = false;
                g_interrupt_roundtrip_confirmed = false;
                g_all_roundtrip_confirmed       = false;

                APP_PRINT("\r\n[USB HVND] PVND device configured.\r\n");

                /* Get USB pipe information. */
                g_usb_on_usb.usedPipesGet(&g_basic0_ctrl, &used_pipe, ADDRESS1);

                for (pipe = START_PIPE; pipe < END_PIPE; pipe++)
                {
                    if ((used_pipe & (1U << pipe)) != 0U)
                    {
                        g_usb_on_usb.pipeInfoGet(&g_basic0_ctrl, &pipe_info, pipe);

                        if (USB_EP_DIR_IN != (pipe_info.endpoint & USB_EP_DIR_IN))
                        {
                            /* Out transfer. */
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
                            /* In transfer. */
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

                APP_PRINT("[USB HVND] Bulk OUT=%u, Bulk IN=%u, Interrupt OUT=%u, Interrupt IN=%u.\r\n",
                          (unsigned int) bulk_out_pipe,
                          (unsigned int) bulk_in_pipe,
                          (unsigned int) int_out_pipe,
                          (unsigned int) int_in_pipe);

                /* Send the first Vendor Class Request. */
                class_request_set_vendor_no_data(&g_basic0_ctrl, event_info.device_address);
                break;
            }

            case USB_STATUS_READ_COMPLETE:
            {
                if (FSP_ERR_USB_FAILED != event_info.status)
                {
                    if (bulk_in_pipe == g_usb_hstd_pipe[event_info.pipe].epnum)
                    {
                        buf_type = BUF_BULK;
                        pipe     = bulk_out_pipe;
                    }
                    else if (int_in_pipe == g_usb_hstd_pipe[event_info.pipe].epnum)
                    {
                        buf_type = BUF_INT;
                        pipe     = int_out_pipe;
                    }
                    else
                    {
                        while (1)
                        {
                            /* Halt on an unexpected pipe. */
                        }
                    }

                    debug_info[debug_cnt]  = 1U;
                    debug_info1[debug_cnt] = pipe;
                    debug_info2[debug_cnt] = (uint8_t) event_info.data_size;
                    debug_cnt++;
                    debug_cnt &= 0x3FU;

                    debug_info[debug_cnt]  = 0x08U;
                    debug_info1[debug_cnt] = (uint8_t) USB_M0->BRDYENB;
                    debug_info2[debug_cnt] = (uint8_t) USB_M0->BRDYSTS;
                    debug_cnt++;
                    debug_cnt &= 0x3FU;

                    /*
                     * The Host has received the payload echoed by PVND.
                     * buffer_check() returns only when every received byte matches
                     * the original test pattern, so this is a round-trip PASS.
                     */
                    buffer_check(buf_type, event_info.data_size);

                    if ((BUF_BULK == buf_type) && (false == g_bulk_roundtrip_confirmed))
                    {
                        APP_PRINT("[PASS] USB HVND bulk round-trip verified: %lu bytes.\r\n",
                                  (unsigned long) event_info.data_size);
                        g_bulk_roundtrip_confirmed = true;
                    }
                    else if ((BUF_INT == buf_type) && (false == g_interrupt_roundtrip_confirmed))
                    {
                        APP_PRINT("[PASS] USB HVND interrupt round-trip verified: %lu bytes.\r\n",
                                  (unsigned long) event_info.data_size);
                        g_interrupt_roundtrip_confirmed = true;
                    }
                    else
                    {
                        /* This transfer type has already been reported. */
                    }

                    if ((true == g_bulk_roundtrip_confirmed) &&
                        (true == g_interrupt_roundtrip_confirmed) &&
                        (false == g_all_roundtrip_confirmed))
                    {
                        APP_PRINT("\r\n[PASS] USB HVND <-> PVND communication verified.\r\n");
                        APP_PRINT("       Bulk 512-byte echo: OK\r\n");
                        APP_PRINT("       Interrupt 64-byte echo: OK\r\n");
                        g_all_roundtrip_confirmed = true;
                    }

                    g_usb_on_usb.pipeWrite(&g_basic0_ctrl,
                                           &g_buf[buf_type][0],
                                           event_info.data_size,
                                           pipe);
                }
                else
                {
                    APP_PRINT("[ERROR] USB HVND read transfer failed. status=%d\r\n",
                              (int) event_info.status);
                }

                break;
            }

            case USB_STATUS_WRITE_COMPLETE:
            {
                if (bulk_out_pipe == g_usb_hstd_pipe[event_info.pipe].epnum)
                {
                    buf_type = BUF_BULK;

                    if (1U == is_zlp[buf_type])
                    {
                        pipe = bulk_in_pipe;
                    }
                }
                else if (int_out_pipe == g_usb_hstd_pipe[event_info.pipe].epnum)
                {
                    buf_type = BUF_INT;

                    if (1U == is_zlp[buf_type])
                    {
                        pipe = int_in_pipe;
                    }
                }
                else
                {
                    /* Nothing to do. */
                }

                if (1U == is_zlp[buf_type])
                {
                    is_zlp[buf_type] = 1U;

                    buffer_clear(buf_type);

                    if (BUF_BULK == buf_type)
                    {
                        g_usb_on_usb.pipeRead(&g_basic0_ctrl,
                                              &g_buf[buf_type][0],
                                              BUF_SIZE,
                                              pipe);
                    }
                    else
                    {
                        g_usb_on_usb.pipeRead(&g_basic0_ctrl,
                                              &g_buf[buf_type][0],
                                              INT_TR_SIZE,
                                              pipe);
                    }

                    debug_info[debug_cnt]  = 1U;
                    debug_info1[debug_cnt] = pipe;
                    debug_info2[debug_cnt] = (uint8_t) BUF_SIZE;
                    debug_cnt++;
                    debug_cnt &= 0x3FU;
                }
                else
                {
                    is_zlp[buf_type] = 1U;

                    /* Send a zero-length packet. */
                    g_usb_on_usb.pipeWrite(&g_basic0_ctrl, NULL, 0U, event_info.pipe);

                    debug_info[debug_cnt]  = 1U;
                    debug_info1[debug_cnt] = pipe;
                    debug_info2[debug_cnt] = 0U;
                    debug_cnt++;
                    debug_cnt &= 0x3FU;
                }

                break;
            }

            case USB_STATUS_REQUEST_COMPLETE:
            {
                if (USB_SET_VENDOR_NO_DATA == (event_info.setup.request_type & USB_BREQUEST))
                {
                    class_request_set_vendor(&g_basic0_ctrl, event_info.device_address);
                }
                else if (USB_SET_VENDOR == (event_info.setup.request_type & USB_BREQUEST))
                {
                    class_request_get_vendor(&g_basic0_ctrl, event_info.device_address);
                }
                else if (USB_GET_VENDOR == (event_info.setup.request_type & USB_BREQUEST))
                {
                    buffer_init();

                    /* Start the Bulk Out transfer. */
                    g_usb_on_usb.pipeWrite(&g_basic0_ctrl,
                                           &g_buf[BUF_BULK][0],
                                           BUF_SIZE,
                                           bulk_out_pipe);

                    debug_info[debug_cnt]  = 1U;
                    debug_info1[debug_cnt] = bulk_out_pipe;
                    debug_info2[debug_cnt] = (uint8_t) (BUF_SIZE - USB_APL_MXPS);
                    debug_cnt++;
                    debug_cnt &= 0x3FU;
                    is_zlp[BUF_BULK] = 1U;

                    /* Start the Interrupt Out transfer. */
                    g_usb_on_usb.pipeWrite(&g_basic0_ctrl,
                                           &g_buf[BUF_INT][0],
                                           INT_TR_SIZE,
                                           int_out_pipe);

                    debug_info[debug_cnt]  = 1U;
                    debug_info1[debug_cnt] = int_out_pipe;
                    debug_info2[debug_cnt] = (uint8_t) (BUF_SIZE - USB_APL_MXPS);
                    debug_cnt++;
                    debug_cnt &= 0x3FU;
                    is_zlp[BUF_INT] = 1U;
                }
                else
                {
                    /* Unsupported request. */
                }

                break;
            }

            case USB_STATUS_DETACH:
            {
                APP_PRINT("\r\n[USB HVND] PVND device detached.\r\n");
                g_bulk_roundtrip_confirmed      = false;
                g_interrupt_roundtrip_confirmed = false;
                g_all_roundtrip_confirmed       = false;
                break;
            }

            default:
            {
                break;
            }
        }
    }
}

/*******************************************************************************
 * End of function main_task
 ******************************************************************************/

/******************************************************************************
 * Function Name: class_request_set_vendor
 * Description  : Send the SET_VENDOR class request.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
static void class_request_set_vendor(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    for (uint16_t i = 0U; i < REQ_SIZE; i++)
    {
        g_request_buf[i] = (uint8_t) i;
    }

    setup.request_type   = SET_VENDOR;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = REQ_SIZE;

    g_usb_on_usb.hostControlTransfer(p_ctrl,
                                     &setup,
                                     &g_request_buf[0],
                                     device_address);
}

/*******************************************************************************
 * End of function class_request_set_vendor
 ******************************************************************************/

/******************************************************************************
 * Function Name: class_request_set_vendor_no_data
 * Description  : Send the SET_VENDOR_NO_DATA class request.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
static void class_request_set_vendor_no_data(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    for (uint16_t i = 0U; i < REQ_SIZE; i++)
    {
        g_request_buf[i] = (uint8_t) i;
    }

    setup.request_type   = SET_VENDOR_NO_DATA;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = 0x0000U;

    g_usb_on_usb.hostControlTransfer(p_ctrl,
                                     &setup,
                                     &g_request_buf[0],
                                     device_address);
}

/*******************************************************************************
 * End of function class_request_set_vendor_no_data
 ******************************************************************************/

/******************************************************************************
 * Function Name: class_request_get_vendor
 * Description  : Send the GET_VENDOR class request.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
static void class_request_get_vendor(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    for (uint16_t i = 0U; i < REQ_SIZE; i++)
    {
        g_request_buf[i] = 0U;
    }

    setup.request_type   = GET_VENDOR;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = REQ_SIZE;

    g_usb_on_usb.hostControlTransfer(p_ctrl,
                                     &setup,
                                     &g_request_buf[0],
                                     device_address);
}

/*******************************************************************************
 * End of function class_request_get_vendor
 ******************************************************************************/

/******************************************************************************
 * Function Name: buffer_init
 * Description  : Initialize the USB transfer buffers.
 * Arguments    : None.
 * Return Value : None.
 ******************************************************************************/
static void buffer_init(void)
{
    for (uint16_t j = 0U; j < 2U; j++)
    {
        for (uint16_t i = 0U; i < BUF_SIZE; i++)
        {
            g_buf[j][i] = (uint8_t) i;
        }
    }
}

/*******************************************************************************
 * End of function buffer_init
 ******************************************************************************/

/******************************************************************************
 * Function Name: buffer_check
 * Description  : Verify received USB transfer data.
 * Arguments    : buf_type -
 *                    Buffer number.
 *                size -
 *                    Number of bytes to verify.
 * Return Value : None.
 ******************************************************************************/
static void buffer_check(uint16_t buf_type, uint32_t size)
{
    for (uint16_t i = 0U; i < (uint16_t) size; i++)
    {
        if ((uint8_t) (i & USB_VALUE_FF) != g_buf[buf_type][i])
        {
            APP_PRINT("[ERROR] USB HVND data mismatch: type=%u byte=%u expected=0x%02X actual=0x%02X.\r\n",
                      (unsigned int) buf_type,
                      (unsigned int) i,
                      (unsigned int) ((uint8_t) (i & USB_VALUE_FF)),
                      (unsigned int) g_buf[buf_type][i]);

            while (1)
            {
                /* Halt on data mismatch. */
            }
        }
    }
}

/*******************************************************************************
 * End of function buffer_check
 ******************************************************************************/

/******************************************************************************
 * Function Name: buffer_clear
 * Description  : Clear the selected USB transfer buffer.
 * Arguments    : buf_type -
 *                    Buffer number.
 * Return Value : None.
 ******************************************************************************/
static void buffer_clear(uint16_t buf_type)
{
    for (uint16_t i = 0U; i < BUF_SIZE; i++)
    {
        g_buf[buf_type][i] = 0U;
    }
}

/*******************************************************************************
 * End of function buffer_clear
 ******************************************************************************/

#if (BSP_CFG_RTOS == 2)
/******************************************************************************
 * Function Name: usb_apl_rec_msg
 * Description  : Receive a message from the specified USB application mailbox.
 * Arguments    : id -
 *                    Mailbox ID.
 *                mess -
 *                    Pointer to a message pointer.
 *                tm -
 *                    Timeout value.
 * Return Value : USB_APL_OK or USB_APL_ERROR.
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

    err = xQueueReceive(handle, (void *) mess, portMAX_DELAY);

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

/*******************************************************************************
 * End of function usb_apl_rec_msg
 ******************************************************************************/

/******************************************************************************
 * Function Name: usb_apl_snd_msg
 * Description  : Send a message to the specified USB application mailbox.
 * Arguments    : id -
 *                    Mailbox ID.
 *                mess -
 *                    Pointer to the USB message.
 * Return Value : USB_APL_OK or USB_APL_ERROR.
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

    err = xQueueSend(handle, (const void *) &mess, (TickType_t) 0);

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

/*******************************************************************************
 * End of function usb_apl_snd_msg
 ******************************************************************************/
#endif /* BSP_CFG_RTOS == 2 */

/******************************************************************************
 * End Of File
 ******************************************************************************/
