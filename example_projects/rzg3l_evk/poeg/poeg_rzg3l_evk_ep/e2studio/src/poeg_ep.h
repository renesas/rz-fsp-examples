/***********************************************************************************************************************
 * File Name    : poeg_ep.h
 * Description  : Contains declarations of data structures and functions used in poeg_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef POEG_EP_H_
#define POEG_EP_H_

#include "common_utils.h"

/* Macros for commands to be received through RTT input */
#define POEG_TRIGGER_ON_GTETRG_INPUT_PIN_LEVEL (1U)
#define POEG_OUTPUT_DISABLE_SOFTWARE_API       (2U)
#define POEG_OUTPUT_DISABLE_REQUEST_FROM_GPT   (3U)
#define POEG_RESET                             (4U)

/* Macro for POEG channels */
#define POEG_CHANNEL_0 (1U)
#define POEG_CHANNEL_3 (2U)

/* Macro for LED pins and status */
#define LED_PIN_POEG0 (BSP_IO_PORT_06_PIN_06)
#define LED_PIN_POEG3 (BSP_IO_PORT_08_PIN_04)
#define SET_FLAG      (1)
#define CLEAR_FLAG    (0)

#define NULL_CHAR   ('\0')

#define EP_INFO "\r\nThe example project demonstrates the basic functionalities of "\
                "\r\nPOEG driver on Renesas RZG MPU based on Renesas FSP. RZG MPU is "\
                "\r\nused to demonstrate the various operating modes of a POEG driver."\
                "\r\nThe example project demonstrates the modes of POEG module:"\
                "\r\n1.POEG Trigger Pin mode-GPT Output disable on GTETRG pin level"\
                "\r\n2.POEG Software API mode-GPT Output disable by software API"\
                "\r\n3.POEG Trigger Output Level mode-GPT Output disable from the GPT"\
                "\r\n4.Enable Port output using POEG_Reset API."\
                "\r\nUsing RTT input, user can choose the POEG method.\r\n"\

#define MENU_INFO   "\r\nPOEG Application Menu\r\n"\
                    "1 :  Disable GPT output pin using POEG trigger pin\r\n"\
                    "2 :  Disable GPT output pin using POEG software API\r\n"\
                    "3 :  Disable GPT output pin using POEG trigger GPT output level\r\n"\
                    "4 :  Reset the POEG Channel\r\n"\
                    "** Reset of application is needed prior to continuous selection of the\r\n"\
                    "menu option 1 **\r\n"\
                    "User Input :  "

#define RESET_MENU_INFO "\r\nPOEG RESET Menu\r\n"\
                        "1 :  RESET POEG Channel 0 \r\n"\
                        "2 :  RESET POEG Channel 3 \r\n"\
                        "User Input :  "

/* Function declarations */
void deinit_gpt_timer (timer_ctrl_t * const p_timer_ctl);
fsp_err_t init_poeg0_module (void);
fsp_err_t init_poeg3_module (void);
fsp_err_t init_gpt_timer_pwm0 (void);
fsp_err_t init_gpt_timer_pwm5 (void);
fsp_err_t deinit_poeg0_module (void);
fsp_err_t deinit_poeg3_module (void);
fsp_err_t poeg_output_disable_and_reset_options (void);
fsp_err_t reset_poeg_module (void);

#endif /* POEG_EP_H_ */
/***********************************************************************************************************************
 * @} (end addtogroup r_poeg_ep)
 **********************************************************************************************************************/
