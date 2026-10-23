/***********************************************************************************************************************
 * File Name    : r_usb_hcdc_example_echo.c
 * Description  : Contains USB Host CDC example processing and echo operation.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "common_utils.h"
#include <r_usb_basic.h>
#include "r_usb_hcdc_api.h"
#include "r_usb_typedef.h"
#include "r_usb_hcdc.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define DATA_LEN                (512U)
#define USB_DEVICE_ADDRESS_1    (1U)
#define SET_LINE_CODING         (USB_CDC_SET_LINE_CODING | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define GET_LINE_CODING         (USB_CDC_GET_LINE_CODING | USB_DEV_TO_HOST | USB_CLASS | USB_INTERFACE)
#define SET_CONTROL_LINE_STATE  (USB_CDC_SET_CONTROL_LINE_STATE | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define COM_SPEED               (9600U)
#define COM_DATA_BIT            (8U)
#define COM_STOP_BIT            (0U)
#define COM_PARITY_BIT          (0U)
#define LINE_CODING_LENGTH      (7U)

/******************************************************************************
 * Typedef definitions
 ******************************************************************************/
typedef struct
{
    usb_setup_t setup;     /* Class-specific notification. */
    uint8_t     bitmap[2]; /* UART state bitmap. */
} serial_state_t;

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static usb_hcdc_linecoding_t g_com_parm;
static usb_hcdc_linecoding_t g_com_parm_temp;
static serial_state_t        g_serial_state;
static uint8_t               g_snd_buf[DATA_LEN];
static uint8_t               g_rcv_buf[DATA_LEN];
static bool                  g_pcdc_receive_confirmed = false;

void usb_basic_example(void);
void set_control_line_state(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
void set_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);
void get_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

/*******************************************************************************
 * Function Name: usb_basic_example
 * Description  : Run the USB Host CDC event loop and echo received data.
 * Arguments    : None.
 * Return Value : None.
 ******************************************************************************/
void usb_basic_example(void)
{
    usb_status_t     event;
    usb_event_info_t event_info;
    uint16_t         i;

    /* Open the USB Host CDC instance. */
    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);

    while (1)
    {
        /* Get USB event data. */
        g_usb_on_usb.eventGet(&event_info, &event);

        /* Handle the received event, if any. */
        switch (event)
        {
            case USB_STATUS_CONFIGURED:
            {
                APP_PRINT("\r\n[USB HCDC] PCDC device configured successfully.\r\n");

                /* Configure virtual UART settings. */
                set_line_coding(&g_basic0_ctrl, event_info.device_address);
                break;
            }

            case USB_STATUS_READ_COMPLETE:
            {
                if (USB_CLASS_HCDC == event_info.type)
                {
                    if (event_info.data_size > 0U)
                    {
                        /*
                         * Confirm that HCDC received actual data from the PCDC device.
                         * The PCDC example echoes the HCDC test pattern, so compare the
                         * received payload with the previously transmitted data.
                         */
                        if ((event_info.data_size <= DATA_LEN) &&
                            (0 == memcmp(g_rcv_buf, g_snd_buf, event_info.data_size)))
                        {
                            if (false == g_pcdc_receive_confirmed)
                            {
                                APP_PRINT("\r\n============================================================\r\n");
                                APP_PRINT("[PASS] PCDC -> HCDC data reception confirmed.\r\n");
                                APP_PRINT("[PASS] Received size : %u bytes.\r\n",
                                          (unsigned int) event_info.data_size);
                                APP_PRINT("[PASS] Payload check : MATCH.\r\n");
                                APP_PRINT("[PASS] USB HCDC <-> PCDC communication is working.\r\n");
                                APP_PRINT("============================================================\r\n");

                                g_pcdc_receive_confirmed = true;
                            }
                        }
                        else
                        {
                            APP_PRINT("\r\n[ERROR] PCDC data received, but payload verification failed.\r\n");
                            APP_PRINT("[ERROR] Received size : %u bytes.\r\n",
                                      (unsigned int) event_info.data_size);
                        }

                        /* Send the received data back to the connected peripheral. */
                        g_usb_on_usb.write(&g_basic0_ctrl,
                                           g_snd_buf,
                                           event_info.data_size,
                                           USB_DEVICE_ADDRESS_1);
                    }
                    else
                    {
                        /* Restart data reception after a zero-length packet. */
                        g_usb_on_usb.read(&g_basic0_ctrl,
                                          g_rcv_buf,
                                          DATA_LEN,
                                          USB_DEVICE_ADDRESS_1);
                    }
                }
                else
                {
                    /* Start receiving the class notification SerialState. */
                    g_usb_on_usb.read(&g_basic0_ctrl,
                                      (uint8_t *) &g_serial_state,
                                      USB_HCDC_SERIAL_STATE_MSG_LEN,
                                      USB_DEVICE_ADDRESS_1);
                }

                break;
            }

            case USB_STATUS_WRITE_COMPLETE:
            {
                /* Start the next receive operation. */
                g_usb_on_usb.read(&g_basic0_ctrl,
                                  g_rcv_buf,
                                  DATA_LEN,
                                  USB_DEVICE_ADDRESS_1);
                break;
            }

            case USB_STATUS_REQUEST_COMPLETE:
            {
                if (USB_CDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Set virtual RTS/DTR signal state. */
                    set_control_line_state(&g_basic0_ctrl, event_info.device_address);
                }
                else if (USB_CDC_SET_CONTROL_LINE_STATE ==
                         (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Read back virtual UART settings. */
                    get_line_coding(&g_basic0_ctrl, event_info.device_address);
                }
                else if (USB_CDC_GET_LINE_CODING ==
                         (event_info.setup.request_type & USB_BREQUEST))
                {
                    /* Setup is complete. Start the loopback operation. */
                    for (i = 0U; i < DATA_LEN; i++)
                    {
                        g_snd_buf[i] = (uint8_t) i;
                    }

                    APP_PRINT("[USB HCDC] Sending %u-byte test pattern to PCDC.\r\n",
                              (unsigned int) DATA_LEN);

                    g_usb_on_usb.write(&g_basic0_ctrl,
                                       g_snd_buf,
                                       DATA_LEN,
                                       USB_DEVICE_ADDRESS_1);
                }
                else
                {
                    /* Unsupported request. */
                }

                break;
            }

            default:
            {
                /* Other event. */
                break;
            }
        }
    }
}

/*******************************************************************************
 * End of function usb_basic_example
 ******************************************************************************/

/*******************************************************************************
 * Function Name: set_control_line_state
 * Description  : Send the CDC SetControlLineState class request.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
void set_control_line_state(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    setup.request_type   = SET_CONTROL_LINE_STATE;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = 0x0000U;

    usb_hcdc_set_control_line_state(p_ctrl, device_address, &setup);
}

/*******************************************************************************
 * End of function set_control_line_state
 ******************************************************************************/

/*******************************************************************************
 * Function Name: set_line_coding
 * Description  : Configure the connected CDC device line-coding parameters.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
void set_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    g_com_parm.dwdte_rate   = (usb_hcdc_line_speed_t) COM_SPEED;
    g_com_parm.bchar_format = (usb_hcdc_stop_bit_t) COM_STOP_BIT;
    g_com_parm.bparity_type = (usb_hcdc_parity_bit_t) COM_PARITY_BIT;
    g_com_parm.bdata_bits   = (usb_hcdc_data_bit_t) COM_DATA_BIT;

    setup.request_type   = SET_LINE_CODING;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = LINE_CODING_LENGTH;

    /* Request control transfer. */
    usb_hcdc_set_line_coding(p_ctrl, device_address, &g_com_parm, &setup);
}

/*******************************************************************************
 * End of function set_line_coding
 ******************************************************************************/

/*******************************************************************************
 * Function Name: get_line_coding
 * Description  : Read the connected CDC device line-coding parameters.
 * Arguments    : p_ctrl -
 *                    Pointer to the USB instance control structure.
 *                device_address -
 *                    Connected USB device address.
 * Return Value : None.
 ******************************************************************************/
void get_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    setup.request_type   = GET_LINE_CODING;
    setup.request_value  = 0x0000U;
    setup.request_index  = 0x0000U;
    setup.request_length = LINE_CODING_LENGTH;

    /* Request control transfer. */
    usb_hcdc_get_line_coding(p_ctrl, device_address, &g_com_parm_temp, &setup);
}

/*******************************************************************************
 * End of function get_line_coding
 ******************************************************************************/
