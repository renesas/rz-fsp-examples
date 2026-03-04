/***********************************************************************************************************************
 * File Name    : usb_hcdc_ep.h
 * Description  : Contains declarations of data structures and functions used in usb_hcdc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef USB_HCDC_EP_H_
#define USB_HCDC_EP_H_

/***********************************************************************************************************************
 *  Function Name: set_control_line_state
 *  Description  : set control line state function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void set_control_line_state (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

/***********************************************************************************************************************
 *  Function Name: set_line_coding
 *  Description  : set line coding function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void set_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

/***********************************************************************************************************************
 *  Function Name: get_line_coding
 *  Description  : get line coding function.
 *  Arguments    : p_ctrl             Pointer to the usb event info arguments
 *                 device_address     Device Address
 *  Return Value : None
 **********************************************************************************************************************/
void get_line_coding (usb_instance_ctrl_t * p_ctrl, uint8_t device_address);

/***********************************************************************************************************************
 * @fn     handle_error 
 * @brief  This function is called to do closing of usb module using its HAL level API and handles the error trap.
 *  Handle the Error internally with Proper Message. Application handles the rest.
 * @param[IN] err       Return values of APIs
 * @param[IN] err_str   Print message from the failed API call
 * @retval    None
 **********************************************************************************************************************/
void handle_error(fsp_err_t err, char *err_str);

#endif /* USB_HCDC_EP_H_ */
