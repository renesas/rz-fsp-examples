/***********************************************************************************************************************
 * File Name    : r_usb_pcdc_echo_apl.c
 * Description  : Contains data structures and functions used in r_usb_pcdc_echo_apl.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/***********************************************************************************************************************
 Includes   <System Includes> , "Project Includes"
 **********************************************************************************************************************/
#include "hal_data.h"
#include "r_usb_basic.h"
#include "r_usb_basic_cfg.h"
#include "r_usb_pcdc_cfg.h"
#include "r_usb_pcdc_apl.h"
#include "r_usb_pcdc_api.h"
#include "r_usb_typedef.h"
#include "r_usb_extern.h"

#if USB_CFG_DTC == USB_CFG_ENABLE
#include "r_dtc_rx_if.h"
#endif /* USB_CFG_DTC == USB_CFG_ENABLE */

#if USB_CFG_DMA == USB_CFG_ENABLE
//#include "r_dmaca_rx_if.h"
#endif /* USB_CFG_DMA == USB_CFG_ENABLE */

#if (BSP_CFG_RTOS == 2)
#include "inc/r_usb_rtos_apl.h"
#endif /* (BSP_CFG_RTOS == 2) */

#ifdef USB_CFG_PCDC_USE

#if OPERATION_MODE == USB_ECHO
/***********************************************************************************************************************
 Macro definitions
 **********************************************************************************************************************/

#define FSP_VECTOR_USB_D0FIFO          ((IRQn_Type)1)

#define FSP_VECTOR_USB_D1FIFO          ((IRQn_Type)2)

#define FSP_VECTOR_USB_USBI            ((IRQn_Type)3)
#define FSP_VECTOR_USB_USBI_IPL        (1U)

#define FSP_VECTOR_USB_USBR            ((IRQn_Type)4)
#define FSP_VECTOR_USB_USBR_IPL        (2U)

#define FSP_VECTOR_USB_USBIR           ((IRQn_Type)5)
#define FSP_VECTOR_USB_USBIR_IPL       (3U)

/***********************************************************************************************************************
 Private global variables and functions
 **********************************************************************************************************************/
extern uint8_t   g_apl_device[];
extern uint8_t   g_apl_configuration[];
extern uint8_t   g_apl_hs_configuration[];
extern uint8_t   g_apl_qualifier_descriptor[];
extern uint8_t * gp_apl_string_table[];

/* const */ usb_descriptor_t g_usb_descriptor =
{
    g_apl_device,                   /* Pointer to the device descriptor */
    g_apl_configuration,            /* Pointer to the configuration descriptor for Full-speed */
    g_apl_hs_configuration,         /* Pointer to the configuration descriptor for Hi-speed */
    g_apl_qualifier_descriptor,     /* Pointer to the qualifier descriptor */
    gp_apl_string_table,             /* Pointer to the string descriptor table */
    NUM_STRING_DESCRIPTOR
};

/***********************************************************************************************************************
Exported global functions (to be accessed by other files)
 **********************************************************************************************************************/
/***********************************************************************************************************************
 Renesas Peripheral Communications Devices Class Sample Code functions
 **********************************************************************************************************************/

#if (BSP_CFG_RTOS == 2)
/***********************************************************************************************************************
 Function Name   : usb_apl_callback
 Description     : Callback function for Application program
 Arguments       : usb_ctrl_t *p_ctrl   : Control structure for USB API.
                   usb_hdl_t  cur_task  : Task Handle
                   uint8_t    usb_state : USB_ON(USB_STATUS_REQUEST) / USB_OFF
 Return value    : none
 **********************************************************************************************************************/
void usb_apl_callback (usb_event_info_t *p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state)
{
    (void)usb_state;
    (void)cur_task;

    USB_APL_SND_MSG(USB_APL_MBX, (usb_msg_t *)p_api_event);
} /* End of function usb_apl_callback */
#endif /* (BSP_CFG_RTOS == 2) */

#if (BSP_CFG_RTOS == 2)
/***********************************************************************************************************************
 Function Name   : usb_apl_rec_msg
 Description     : Receive a message to the specified id (mailbox).
 Argument        : uint8_t id        : ID number (mailbox).
                 : usb_msg_t** mess  : Message pointer
                 : usb_tm_t tm       : Timeout Value
 Return          : uint16_t          : USB_OK / USB_ERROR
 **********************************************************************************************************************/
usb_er_t usb_apl_rec_msg (uint8_t id, usb_msg_t** mess, usb_tm_t tm)
{
    BaseType_t err;
    QueueHandle_t handle;
    usb_er_t result;

    (void)tm;

    if (NULL == mess)
    {
        return USB_APL_ERROR;
    }

    handle = (*(g_apl_mbx_table[id]));
    *mess  = NULL;

    /** Receive message from queue specified by *(mbx_table[id]) **/
    err = xQueueReceive(handle, (void *)mess,(portMAX_DELAY));
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
/***********************************************************************************************************************
 End of function usb_apl_rec_msg
 **********************************************************************************************************************/
/***********************************************************************************************************************
 Function Name   : usb_apl_snd_msg
 Description     : Send a message to the specified id (mailbox).
 Argument        : uint8_t id        : ID number (mailbox).
                 : usb_msg_t* mess   : Message pointer
 Return          : usb_er_t          : USB_OK / USB_ERROR
 **********************************************************************************************************************/
usb_er_t usb_apl_snd_msg (uint8_t id, usb_msg_t* mess)
{
    BaseType_t err;
    QueueHandle_t handle;
    usb_er_t result;

    if (NULL == mess)
    {
        return USB_APL_ERROR;
    }

    handle = (*(g_apl_mbx_table[id]));

    /** Send message to queue specified by *(mbx_table[id]) **/
    err = xQueueSend(handle, (const void *)&mess, (TickType_t)(0));
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
/***********************************************************************************************************************
 End of function usb_apl_snd_msg
 **********************************************************************************************************************/
#endif /* #if (BSP_CFG_RTOS == 2) */
#endif  /* OPERATION_MODE == USB_ECHO */
#endif /* USB_CFG_PCDC_USE */
/***********************************************************************************************************************
 End  Of File
 **********************************************************************************************************************/
