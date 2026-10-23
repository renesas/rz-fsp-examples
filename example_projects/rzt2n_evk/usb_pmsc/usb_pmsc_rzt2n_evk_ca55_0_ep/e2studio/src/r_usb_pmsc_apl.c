/***********************************************************************************************************************
 * File Name    : r_usb_pmsc_apl.c
 * Description  : Contains USB Peripheral Mass Storage Class application processing.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

#include "r_usb_basic.h"
#include "r_usb_basic_cfg.h"
#include "r_usb_pmsc_apl.h"
#include "r_usb_pmsc_api.h"
#include "rm_block_media_api.h"
#include "bsp_pin_cfg.h"
#include "r_usb_typedef.h"
#include "common_data.h"
#include "common_utils.h"

#if (BSP_CFG_RTOS == 2)
#include "usb_pmsc_thread.h"
#endif

/******************************************************************************
 Macro definitions
 ******************************************************************************/
#if (BSP_CFG_RTOS == 2)
#define USB_APL_MBX (USB_TID_10)
#endif /* (BSP_CFG_RTOS == 2) */

/******************************************************************************
 Private global variables and functions
 ******************************************************************************/
void main_task(void);

extern uint8_t g_apl_device[];
extern uint8_t g_apl_configuration[];
extern uint8_t g_apl_hs_configuration[];
extern uint8_t g_apl_qualifier_descriptor[];
extern uint8_t *g_apl_string_table[];
volatile uint8_t configured_flag = 0U;

#if (BSP_CFG_RTOS == 2)
extern QueueHandle_t g_apl_mbx_hdl;
#endif

const usb_descriptor_t g_usb_descriptor =
{
    g_apl_device,
    g_apl_configuration,
    g_apl_hs_configuration,         /* Pointer to the configuration descriptor for Hi-speed */
    g_apl_qualifier_descriptor,     /* Pointer to the qualifier descriptor */
    g_apl_string_table,
    NUM_STRING_DESCRIPTOR
};

extern const rm_block_media_api_t g_rm_block_media_on_user_media;

extern void r_usb_pmsc_block_media_event_callback(rm_block_media_callback_args_t * p_args);

const rm_block_media_cfg_t g_rm_block_media0_cfg =
{
    .p_extend   = NULL,
    .p_callback = r_usb_pmsc_block_media_event_callback,
    .p_context  = NULL,
};

rm_block_media_instance_t g_rm_block_media0 =
{
    .p_api  = &g_rm_block_media_on_user_media,
    .p_ctrl = NULL,
    .p_cfg  = &g_rm_block_media0_cfg,
};

/******************************************************************************
 Function Name   : main_task
 Description     : Peripheral CDC application main process
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void main_task(void)
{
#if (BSP_CFG_RTOS == 2)
    usb_event_info_t  *p_mess;
#endif
    usb_status_t       event;
    usb_event_info_t   event_info;

    g_usb_on_usb.open(&g_basic0_ctrl, &g_basic0_cfg);
    APP_PRINT("[INFO] USB PMSC driver opened.\r\n");
    APP_PRINT("[INFO] Waiting for host enumeration...\r\n");

    while (1)
    {
#if (BSP_CFG_RTOS == 2)
        xQueueReceive(g_apl_mbx_hdl, (void *) &p_mess, portMAX_DELAY);
        event_info = *p_mess;
        event      = event_info.event;
#else   /* (BSP_CFG_RTOS == 2) */
        R_USB_EventGet(&event_info, &event);
#endif
        switch (event)
        {
            case USB_STATUS_CONFIGURED :
                configured_flag = 1U;
                APP_PRINT("[PASS] USB PMSC configured by host.\r\n");
                APP_PRINT("[PASS] RAM disk is available to the host as USB mass storage.\r\n");
            break;
            case USB_STATUS_SUSPEND :
            case USB_STATUS_DETACH :
                configured_flag = 0U;

                if (USB_STATUS_SUSPEND == event)
                {
                    APP_PRINT("[INFO] USB PMSC suspended by host.\r\n");
                }
                else
                {
                    APP_PRINT("[INFO] USB PMSC detached from host.\r\n");
                }

#if USB_SUPPORT_LPW == USB_APL_ENABLE
//@@                low_power_mcu();
#endif
            break;
            default :
            break;
        }
    }
}

#if (BSP_CFG_RTOS == 2)
/******************************************************************************
 Function Name   : usb_apl_callback
 Description     : Callback function for Application program
 Arguments       : usb_ctrl_t *p_ctrl   : Control structure for USB API.
                   usb_hdl_t  cur_task  : Task Handle
                   uint8_t    usb_state : USB_ON(USB_STATUS_REQUEST) / USB_OFF
 Return value    : none
 ******************************************************************************/
void usb_apl_callback (usb_event_info_t *p_api_event, usb_hdl_t cur_task, usb_onoff_t usb_state)
{
    FSP_PARAMETER_NOT_USED (cur_task);
    FSP_PARAMETER_NOT_USED (usb_state);
    xQueueSend(g_apl_mbx_hdl, (const void *) &p_api_event, (TickType_t) (0));
}
#endif
