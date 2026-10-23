/***********************************************************************************************************************
 * File Name    : r_usb_hhid_example.c
 * Description  : Contains USB Host HID example processing and received-data logging.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include <r_usb_basic.h>
#include "r_usb_hhid_api.h"
#include "r_usb_hhid.h"
#include "sio_char.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define HID_PIPELINE
#define SET_PROTOCOL               (USB_HID_SET_PROTOCOL | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define BOOT_PROTOCOL              (0U)
#define BUFSIZE                    (4U * 1024U)
#define USB_RCV_BUF_SIZE           (8U)
#define USB_FS_DEVICE_ADDRESS_1    (1U)

#ifdef HID_PIPELINE
 #define FSP_VECTOR_USB_USBIR_IPL  (3U)
 #define FSP_VECTOR_USB_D0FIFO     ((IRQn_Type) 0)
 #define FSP_VECTOR_USB_D1FIFO     ((IRQn_Type) 1)
 #define FSP_VECTOR_USB_USBI       ((IRQn_Type) 2)
 #define FSP_VECTOR_USB_USBI_IPL   (3U)
 #define FSP_VECTOR_USB_USBR       ((IRQn_Type) 3)
 #define FSP_VECTOR_USB_USBR_IPL   (3U)
 #define FSP_VECTOR_USB_USBIR      ((IRQn_Type) 5)
#endif

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static void set_protocol(usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address);
static void output_log(void);
static uint16_t hid_memcpy(uint8_t * p_dest, uint8_t * p_src, uint16_t offset, uint16_t size);

void r_usb_hhid_example(void);
void user_uart_callback(uart_callback_args_t * p_args);

#if (BSP_CFG_RTOS == 2)
void usb_apl_callback(usb_event_info_t * p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state);
#endif

/******************************************************************************
 * Exported global variables
 ******************************************************************************/
static uint8_t  g_buf[USB_RCV_BUF_SIZE];
static uint8_t  g_hid_type;
static uint32_t g_setup_data[32];
static uint16_t g_mxps;
static uint8_t  g_store_buf[BUFSIZE] USB_BUFFER_PLACE_IN_SECTION;

volatile uint32_t g_transfer_complete = 0U;
volatile uint32_t g_receive_complete  = 0U;

extern const usb_cfg_t g_basic0_cfg;

uint8_t  g_out_of_band_received[TRANSFER_LENGTH];
uint32_t g_out_of_band_index = 0U;

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
 * Function Name: r_usb_hhid_example
 * Description  : Run the USB Host HID application and log received HID data.
 * Arguments    : None.
 * Return Value : None.
 ******************************************************************************/
void r_usb_hhid_example(void)
{
#if (BSP_CFG_RTOS == 2)
    usb_event_info_t * p_mess;
#endif

    usb_status_t     event;
    usb_event_info_t event_info;
    uint16_t         offset = 0U;

    R_USB_Open(&g_basic0_ctrl, &g_basic0_cfg);

    while (1)
    {
#if (BSP_CFG_RTOS == 2)
        USB_APL_RCV_MSG(USB_APL_MBX, (usb_msg_t **) &p_mess);
        event_info = *p_mess;
        event      = event_info.event;
#else
        /* Get the USB event code. */
        R_USB_EventGet(&event_info, &event);
#endif

        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            {
                R_USB_HHID_TypeGet(&g_basic0_ctrl,
                                   &g_hid_type,
                                   USB_FS_DEVICE_ADDRESS_1);

                R_USB_HHID_MaxPacketSizeGet(&g_basic0_ctrl,
                                            &g_mxps,
                                            USB_HID_IN,
                                            USB_FS_DEVICE_ADDRESS_1);

                /* Send the HID SetProtocol request to the connected HID device. */
                set_protocol(&g_basic0_ctrl,
                             BOOT_PROTOCOL,
                             USB_FS_DEVICE_ADDRESS_1);
                break;
            }

            case USB_STATUS_READ_COMPLETE:
            {
                offset = hid_memcpy(g_store_buf, g_buf, offset, g_mxps);

                R_USB_Read(&g_basic0_ctrl,
                           g_buf,
                           (uint32_t) g_mxps,
                           USB_FS_DEVICE_ADDRESS_1);

                output_log();
                break;
            }

            case USB_STATUS_REQUEST_COMPLETE:
            {
                if (USB_HID_SET_PROTOCOL == (event_info.setup.request_type & USB_BREQUEST))
                {
                    R_USB_Read(&g_basic0_ctrl,
                               g_buf,
                               (uint32_t) g_mxps,
                               USB_FS_DEVICE_ADDRESS_1);

                    output_log();
                }

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
 * End of function r_usb_hhid_example
 ******************************************************************************/

/******************************************************************************
 * Function Name: set_protocol
 * Description  : Send the SetProtocol request to the connected HID device.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                protocol -
 *                    HID protocol type.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
static void set_protocol(usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address)
{
    usb_setup_t setup;

    setup.request_type   = SET_PROTOCOL;
    setup.request_value  = protocol;
    setup.request_index  = 0x0000U;
    setup.request_length = 0x0000U;

    R_USB_HostControlTransfer(p_ctrl,
                              &setup,
                              (uint8_t *) &g_setup_data,
                              device_address);
}

/*******************************************************************************
 * End of function set_protocol
 ******************************************************************************/

/******************************************************************************
 * Function Name: hid_memcpy
 * Description  : Copy received HHID data to the application buffer.
 * Arguments    : p_dest -
 *                    Pointer to the application buffer.
 *                p_src -
 *                    Pointer to the received buffer.
 *                offset -
 *                    Application buffer offset.
 *                size -
 *                    Size of received HHID data.
 * Return Value : Updated application buffer offset.
 ******************************************************************************/
static uint16_t hid_memcpy(uint8_t * p_dest, uint8_t * p_src, uint16_t offset, uint16_t size)
{
    uint16_t i;

    for (i = 0U; i < size; i++)
    {
        if ((offset + i) == BUFSIZE)
        {
            offset = 0U;
        }

        *(p_dest + offset + i) = *(p_src + i);
    }

    return (uint16_t) (offset + i);
}

/*******************************************************************************
 * End of function hid_memcpy
 ******************************************************************************/

/******************************************************************************
 * Function Name: output_log
 * Description  : Print the latest received HID data through the existing UART console.
 * Arguments    : None.
 * Return Value : None.
 ******************************************************************************/
static void output_log(void)
{
    APP_PRINT("HID data :");

    for (int32_t i = 0; i < (int32_t) USB_RCV_BUF_SIZE; i++)
    {
        APP_PRINT(" %3u,", g_buf[i]);
    }

    APP_PRINT("\r\n");
}

/*******************************************************************************
 * End of function output_log
 ******************************************************************************/

/******************************************************************************
 * Function Name: user_uart_callback
 * Description  : Handle UART receive and transfer-complete events.
 * Arguments    : p_args -
 *                    Pointer to UART callback arguments.
 * Return Value : None.
 ******************************************************************************/
void user_uart_callback(uart_callback_args_t * p_args)
{
    switch (p_args->event)
    {
        case UART_EVENT_RX_CHAR:
        {
            if (sizeof(g_out_of_band_received) > g_out_of_band_index)
            {
                if ((UART_DATA_BITS_7 == g_uart0_cfg.data_bits) ||
                    (UART_DATA_BITS_8 == g_uart0_cfg.data_bits))
                {
                    g_out_of_band_received[g_out_of_band_index++] = (uint8_t) p_args->data;
                }
#if (TRANSFER_LENGTH >= 2)
                else if ((sizeof(g_out_of_band_received) - g_out_of_band_index) >= sizeof(uint16_t))
                {
                    uint16_t received_data = (uint16_t) p_args->data;

                    memcpy(&g_out_of_band_received[g_out_of_band_index],
                           &received_data,
                           sizeof(received_data));

                    g_out_of_band_index += sizeof(received_data);
                }
#endif
                else
                {
                    /* Ignore data that does not fit in the receive buffer. */
                }
            }

            break;
        }

        case UART_EVENT_RX_COMPLETE:
        {
            g_receive_complete = 1U;
            break;
        }

        case UART_EVENT_TX_COMPLETE:
        {
            g_transfer_complete = 1U;
            break;
        }

        default:
        {
            break;
        }
    }
}

/*******************************************************************************
 * End of function user_uart_callback
 ******************************************************************************/

/******************************************************************************
 * End Of File
 ******************************************************************************/
