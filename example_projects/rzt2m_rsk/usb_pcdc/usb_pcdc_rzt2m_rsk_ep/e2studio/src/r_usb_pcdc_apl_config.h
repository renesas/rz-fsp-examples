/***********************************************************************************************************************
 * File Name    : r_usb_pcdc_apl_config.h
 * Description  : USB Peripheral CDC program configuration file.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef R_USB_PCDC_APL_CONFIG_H
#define R_USB_PCDC_APL_CONFIG_H

/***********************************************************************************************************************
 User define macro definitions
 **********************************************************************************************************************/
/** [USB module selection setting]
 *  USB_IP0             : Uses USB0 module
 *  USB_IP1             : Uses USB1 module
 */
#define USE_USBIP           (USB_IP0)

/** [USB connect speed setting]
 *  USB_HS              : Support Hi-Speed
 *  USB_FS              : Not support Hi-Speed(Full-Speed only)
 */
#define USB_SUPPORT_SPEED   (USB_FS)

/** [Select PCDC Demo sample Application mode]
 *  USB_ECHO              : Loop back(Echo) mode
 *  USB_UART              : USB Serial(VCOM) converter mode
 */
#define OPERATION_MODE  (USB_ECHO)

/** [Select Support Free RTOS]
 *  USB_APL_ENABLE        : Support Free RTOS
 *  USB_APL_DISABLE       : Not support Free RTOS(Support non OS)
 */
#define USB_SUPPORT_RTOS    (USB_APL_DISABLE)

/** [Select USE power saving control]
 *  USB_APL_ENABLE        : Use power saving control
 *  USB_APL_DISABLE       : Not use power saving control
 */
#define USB_SUPPORT_LPW    (USB_APL_DISABLE)

#endif  /* R_USB_PCDC_APL_CONFIG_H */
/***********************************************************************************************************************
 End  Of File
 **********************************************************************************************************************/
