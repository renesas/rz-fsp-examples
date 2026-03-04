/***********************************************************************************************************************
 * File Name    : mtu3_poe3_ep.h
 * Description  : Contains declarations of data structures and functions used in mtu3_poe3_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef MTU3_POE3_H_
#define MTU3_POE3_H_

/* macro definition */
/* Value to stop MTU counter */
#define MTU_COUNT_STOP        (0) 

/* Initial value for complementary PWM start */
#define MTU_CMPLMT_PWM_START  (0xC0)

/* cycle calculate */
#define MTU_C_SET_CYCLE       (0x1388)   /* Full carrier cycle = 100us (PCLKC/4) */
#define MTU_C_CYCLE           (0x9C4)   /* Half carrier cycle = 50us (PCLKC/4) */
#define MTU_DEAD_TIME         (0x64)    /* Dead time = 2us (PCLKC/4) */
/* Duty cycle with dead time compensation */
#define MTU_DUTY_75           ((uint16_t) MTU_C_CYCLE * 1/4+ MTU_DEAD_TIME / 2) /* cast to uint16_t type */
#define MTU_DUTY_50           ((uint16_t) MTU_C_CYCLE * 2/4+ MTU_DEAD_TIME / 2) /* cast to uint16_t type */
#define MTU_DUTY_25           ((uint16_t) MTU_C_CYCLE * 3/4+ MTU_DEAD_TIME / 2) /* cast to uint16_t type */
/* Maximum number of duty cycle steps (25%, 50%, 75%) */
#define DUTY_CNT_MAX          (3)

/* Switch status value */
#define MAIN_SW_STATUS_ON     (0u)
#define MAIN_SW_STATUS_OFF    (1u)

/* Input pin for switch */
#define MAIN_SW1_NB           (R_PORT_NSR->PIN_b[10].PIN5)
#define MAIN_SW2_NB           (R_PORT_NSR->PIN_b[16].PIN3)

/* LED */
#define LED_0                 (0u)
#define LED_1                 (1u)

/* register MSTPCRA (Module Stop Control Register A) */ 
#define MSTPCRA_ADDR          (0x80280308u)

#define EP_INFO     "\nThis sample program demonstrates how to generate three-phase complementary PWM" \
                    "\r\nsignals using the Multi-function Timer Pulse Unit 3 (MTU3) and Unit 4 (MTU4) on the" \
                    "\r\nRZ/T2M micro controller. The program utilizes the MTU3 Three-Phase driver to " \
                    "\r\nconfigure and control the PWM outputs. The Port Output Enable 3 (POE3) module" \
                    "\r\nis used to disable all PWM outputs, effectively stopping the program." \
                    "\r\n" \
                    "\r\nThe main goal is to output six PWM signals (3 positive-phase and 3 negative-phase)" \
                    "\r\nwith a carrier cycle of 100 microsec and a dead-time of 2 microseconds between complementary pairs." \
                    "\r\nThe duty cycle of the PWM signals can be dynamically changed to 25, 50, or 75 by SW2.\r\n" \
                    "\r\nAdditionally, SW1 triggers POE3 to disable all PWM outputs."\
                    "\r\nAny API or event failure message is displayed via RTT Viewer.\r\n\r\n"

/*
 *  Global functions prototype
 */
fsp_err_t init_mtu3_three_phase_and_poe3 (void);
fsp_err_t deinit_mtu3_three_phase_and_poe3 (void);

void r_main_mtu_chng_duty (void);
void set_led (uint8_t LED);
void clear_led (void);

#endif /* MTU3_POE3_H_ */
