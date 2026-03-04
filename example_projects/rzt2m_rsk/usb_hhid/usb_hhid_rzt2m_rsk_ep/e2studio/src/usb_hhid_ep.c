/***********************************************************************************************************************
 * File Name    : usb_hhid_ep.c
 * Description  : Contains data structures and functions used in usb_hhid_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "hal_data.h"
#include "r_usb_basic.h"
#include "r_usb_hhid_api.h"
#include "r_usb_hhid.h"
#include <stdio.h>
#include <stdint.h>
#include "usb_hhid_ep.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
uint8_t  g_buf[USB_RCV_BUF_SIZE];              /* USB Receive data */

static uint32_t s_setup_data[32];

#if (BSP_CFG_RTOS == 2)
/***********************************************************************************************************************
 * Function Name   : usb_apl_callback
 * Description     : Callback function for Application program
 * Arguments       : usb_ctrl_t *p_ctrl   : Control structure for USB API.
 *                 usb_hdl_t  cur_task  : Task Handle
 *                 uint8_t    usb_state : USB_ON(USB_STATUS_REQUEST) / USB_OFF
 * Return value    : none
 **********************************************************************************************************************/
void usb_apl_callback (usb_event_info_t * p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state)
{
    (void) usb_state;
    (void) cur_task;

    USB_APL_SND_MSG(USB_APL_MBX, (usb_msg_t *) p_api_event);
} /* End of function usb_apl_callback */
#endif

/***********************************************************************************************************************
 * Function Name   : set_protocol
 * Description     : Sending SetProtocol request to HID device
 * Arguments       : usb_ctrl_t   *p_ctrl : Pointer to usb_instance_ctrl_t structure.
 *                 : uint8_t      ptorocol: Protocol Type
 *                 : uint8_t      device_address: Device address that sends this request
 * Return value    : none
 **********************************************************************************************************************/
fsp_err_t set_protocol(usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address)
{
    fsp_err_t err = FSP_SUCCESS;

    usb_setup_t setup;

    setup.request_type   = SET_PROTOCOL;      /* bRequestCode:SET_PROTOCOL, bmRequestType */
    setup.request_value  = protocol;          /* wValue: Protocol Type */
    setup.request_index  = 0x0000;            /* wIndex:Interface */
    setup.request_length = 0x0000;            /* wLength:Zero */
/* Request Control transfer */
    err = R_USB_HostControlTransfer(p_ctrl, &setup, (uint8_t *) &s_setup_data, device_address);

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("** R_USB_HostControlTransfer API FAILED **\r\n");
    }

    return err;
}
/***********************************************************************************************************************
 * End of function: set_protocol
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name   : get_report
 * Description     : Sending SetProtocol request to HID device
 * Arguments       : usb_ctrl_t   *p_ctrl : Pointer to usb_instance_ctrl_t structure.
 *                 : uint8_t      ptorocol: Protocol Type
 *                 : uint8_t      device_address: Device address that sends this request
 * Return value    : none
 **********************************************************************************************************************/
void get_report(usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address)
{
    fsp_err_t err = FSP_SUCCESS;

    usb_setup_t setup;

    protocol = 0x1;

    setup.request_type   = GET_REPORT;        /* bRequestCode:SET_PROTOCOL, bmRequestType */
    setup.request_value  = protocol;          /* wValue: Protocol Type */
    setup.request_index  = 0x0000;            /* wIndex:Interface */
    setup.request_length = 65U;               /* wLength:Zero */


    err = R_USB_HostControlTransfer(p_ctrl, &setup,
            (uint8_t *) &s_setup_data, device_address); /* cast to uint8_t* */

    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("** R_USB_HostControlTransfer API FAILED **\r\n");
    }
}
/***********************************************************************************************************************
 * End of function: get_report
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name   : hid_memcpy
 * Description     : Copy received hhid data to the application buffer
 * Arguments       : uint8_t      *p_dest   : Pointer to application buffer
 *                 : uint8_t      *p_src    : Pointer to received buffer
 *                 : uint16_t     offset    : Application buffer offset
 *                 : uint16_t     size      : Size of receiced hhid data
 * Return value    : uint16_t     offset + i: Offset
 **********************************************************************************************************************/
uint16_t hid_memcpy(uint8_t * p_dest, uint8_t * p_src, uint16_t offset, uint16_t size)
{
    uint16_t i;
    for (i = 0; i < size; i++)
    {
        if ((offset + i) == BUFSIZE)
        {
            offset = 0;
        }

        *(p_dest + offset + i) = *(p_src + i);
    }

    return (uint16_t) (offset + i);   /* case to uint16_t */
}
/***********************************************************************************************************************
 * End of function: hid_memcpy
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: output_log
 * Description  : prints all values from the HID receive buffer
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void output_log(void)
{
    APP_PRINT("HID data :");
    for (uint32_t i = 0; i < USB_RCV_BUF_SIZE; i++)
    {
        APP_PRINT(" %3u,", g_buf[i]);
    }
    APP_PRINT("\n");
}
/***********************************************************************************************************************
 * End of function: output_log
 **********************************************************************************************************************/
