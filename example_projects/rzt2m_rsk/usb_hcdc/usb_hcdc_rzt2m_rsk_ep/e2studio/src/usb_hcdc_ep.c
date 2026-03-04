/***********************************************************************************************************************
 * File Name    : usb_hcdc_ep.c
 * Description  : Contains data structures and functions used in usb_hcdc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "r_usb_basic.h"
#include "r_usb_hcdc_api.h"
#include "r_usb_typedef.h"
#include "r_usb_hcdc.h"
#include "usb_hcdc_ep.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SET_LINE_CODING             (USB_CDC_SET_LINE_CODING | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define GET_LINE_CODING             (USB_CDC_GET_LINE_CODING | USB_DEV_TO_HOST | USB_CLASS | USB_INTERFACE)
#define SET_CONTROL_LINE_STATE      (USB_CDC_SET_CONTROL_LINE_STATE | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define COM_SPEED                   (9600U)
#define COM_DATA_BIT                (8U)
#define COM_STOP_BIT                (0)
#define COM_PARITY_BIT              (0)
#define LINE_CODING_LENGTH          (7)

/***********************************************************************************************************************
 * Private global variables and functions
 **********************************************************************************************************************/
usb_hcdc_linecoding_t g_com_parm;           /* Set Line Coding parameter */
usb_hcdc_linecoding_t g_com_parm_temp;      /* Set Line Coding parameter */
/***********************************************************************************************************************
 *  Function Name: set_control_line_state
 *  Description  : set control line state function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void set_control_line_state(usb_instance_ctrl_t *p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    setup.request_type   = SET_CONTROL_LINE_STATE; /* bRequestCode:SET_CONTROL_LINE_STATE, bmRequestType */
    setup.request_value  = 0x0000;                 /* wValue:Zero */
    setup.request_index  = 0x0000;                 /* wIndex:Interface */
    setup.request_length = 0x0000;                 /* wLength:Zero */

    usb_hcdc_set_control_line_state(p_ctrl, device_address, &setup);
}
/***********************************************************************************************************************
 * End of function set_control_line_state
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  Function Name: set_line_coding
 *  Description  : set line coding function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void set_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    g_com_parm.dwdte_rate   = (usb_hcdc_line_speed_t) COM_SPEED;       /* cast usb_hcdc_line_speed_t */
    g_com_parm.bchar_format = (usb_hcdc_stop_bit_t) COM_STOP_BIT;      /* cast usb_hcdc_stop_bit_t */
    g_com_parm.bparity_type = (usb_hcdc_parity_bit_t) COM_PARITY_BIT;  /* cast usb_hcdc_parity_bit_t */
    g_com_parm.bdata_bits   = (usb_hcdc_data_bit_t) COM_DATA_BIT;      /* cast usb_hcdc_data_bit_t */

    setup.request_type   = SET_LINE_CODING;    /* bRequestCode:SET_LINE_CODING, bmRequestType */
    setup.request_value  = 0x0000;             /* wValue:Zero */
    setup.request_index  = 0x0000;             /* wIndex:Interface */
    setup.request_length = LINE_CODING_LENGTH; /* Data:Line Coding Structure */
/* Request Control transfer */
    usb_hcdc_set_line_coding (p_ctrl, device_address, &g_com_parm, &setup);
}
/***********************************************************************************************************************
 * End of function set_line_coding
 **********************************************************************************************************************/
/***********************************************************************************************************************
 *  Function Name: get_line_coding
 *  Description  : get line coding function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void get_line_coding(usb_instance_ctrl_t * p_ctrl, uint8_t device_address)
{
    usb_setup_t setup;

    setup.request_type   = GET_LINE_CODING;    /* bRequestCode:GET_LINE_CODING, bmRequestType */
    setup.request_value  = 0x0000;             /* wValue:Zero */
    setup.request_index  = 0x0000;             /* wIndex:Interface */
    setup.request_length = LINE_CODING_LENGTH; /* Data:Line Coding Structure */
/* Request Control transfer */
    usb_hcdc_get_line_coding (p_ctrl, device_address, &g_com_parm_temp, &setup);
}
/***********************************************************************************************************************
 * End of function get_line_coding
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * @fn     handle_error 
 * @brief  This function is called to do closing of usb module using its HAL level API and handles the error trap.
 *  Handle the Error internally with Proper Message. Application handles the rest.
 * @param[IN] err       Return values of APIs
 * @param[IN] err_str   Print message from the failed API call
 * @retval    None
 **********************************************************************************************************************/
void handle_error(fsp_err_t err, char *err_str)
{
    fsp_err_t error = FSP_SUCCESS;

    /* close opened USB module */
    error = R_USB_Close(&g_basic0_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != error)
    {
        APP_ERR_PRINT ("**\r\n R_USB_Close API FAILED **\r\n");
    }
    APP_ERR_PRINT(err_str);
    APP_ERR_TRAP(err);
}
/***********************************************************************************************************************
 * End of function handle_error
 **********************************************************************************************************************/
