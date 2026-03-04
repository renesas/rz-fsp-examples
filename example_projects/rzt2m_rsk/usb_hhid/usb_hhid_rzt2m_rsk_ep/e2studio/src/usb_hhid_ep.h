/***********************************************************************************************************************
 * File Name    : usb_hhid_ep.h
 * Description  : Contains declarations of data structures and functions used in usb_hhid_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef USB_HHID_H_
#define USB_HHID_H_

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define HID_PIPELINE

#define SET_PROTOCOL               (USB_HID_SET_PROTOCOL | USB_HOST_TO_DEV | USB_CLASS | USB_INTERFACE)
#define GET_REPORT                 (USB_HID_GET_REPORT| USB_DEV_TO_HOST | USB_CLASS | USB_INTERFACE)
#define BOOT_PROTOCOL              (0)
#define BUFSIZE                    (4 * 1024)
#define USB_RCV_BUF_SIZE           (8)
#define USB_FS_DEVICE_ADDRESS_1    (1)

#ifdef HID_PIPELINE
    #define FSP_VECTOR_USB_USBIR_IPL    (3U)
    #define FSP_VECTOR_USB_D0FIFO       ((IRQn_Type) 0)
    #define FSP_VECTOR_USB_D1FIFO       ((IRQn_Type) 1)
    #define FSP_VECTOR_USB_USBI         ((IRQn_Type) 2)
    #define FSP_VECTOR_USB_USBI_IPL     (3U)
    #define FSP_VECTOR_USB_USBR         ((IRQn_Type) 3)
    #define FSP_VECTOR_USB_USBR_IPL     (3U)
    #define FSP_VECTOR_USB_USBIR        ((IRQn_Type) 5)
#endif

void output_log (void);
fsp_err_t set_protocol (usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address);
void get_report (usb_instance_ctrl_t * p_ctrl, uint8_t protocol, uint8_t device_address);

uint16_t hid_memcpy (uint8_t * p_dest, uint8_t * p_src, uint16_t offset, uint16_t size);

#endif /* USB_HHID_H_ */
