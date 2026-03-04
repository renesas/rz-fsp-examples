/***********************************************************************************************************************
 * File Name    : mtu3_poe3_ep.c
 * Description  : Contains data structures and functions used in mtu3_poe3_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "common_utils.h"
#include "mtu3_poe3_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_mtu3_ep
 * @{
 **********************************************************************************************************************/

extern bsp_leds_t g_bsp_leds;

/*
 * Private Global Variables
 */
volatile uint32_t * const gp_mstpcra = (volatile uint32_t *)MSTPCRA_ADDR;

uint16_t g_duty_rate[DUTY_CNT_MAX] = { MTU_DUTY_25, MTU_DUTY_50, MTU_DUTY_75 };
uint8_t  g_duty_cnt;

static three_phase_duty_cycle_t s_duty_cycle;

/*
 * private functions prototype
 */
/***********************************************************************************************************************
 * @brief     Initialize MTU3 three phase driver and POE3 driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       Update duty cycle values for MTU3 three phase,
 *                              MTU3 three phase driver and POE3 driver opened successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_mtu3_three_phase_and_poe3(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* MSTPCRA register Bit 0 Clear to release from the module-stop state */
    (*gp_mstpcra) &= (~((uint32_t)1 << 0));    /* cast to uint32_t type */

    /* Open MTU3 THREE PHASE driver */
    err = R_MTU3_THREE_PHASE_Open(&g_three_phase_ctrl, &g_three_phase_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_THREE_PHASE_Open API failed \r\n");
        return err;
    }

    /* Open POE3 driver */
    err = R_POE3_Open(&g_poe3_ctrl, &g_poe3_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_POE3_Open API failed \r\n");
        return err;
    }

    /* Update duty cycle values (example) */
    s_duty_cycle.duty[THREE_PHASE_CHANNEL_U] = g_duty_rate[0];
    s_duty_cycle.duty[THREE_PHASE_CHANNEL_V] = g_duty_rate[0];
    s_duty_cycle.duty[THREE_PHASE_CHANNEL_W] = g_duty_rate[0];

    err = R_MTU3_THREE_PHASE_DutyCycleSet(&g_three_phase_ctrl, &s_duty_cycle);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_THREE_PHASE_DutyCycleSet API failed \r\n");
        return err;
    }
    
    return err;
}

/***********************************************************************************************************************
 * @brief     Deinitialize MTU3 three phase driver and POE3 driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       MTU3 three phase driver and POE3 driver close successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t deinit_mtu3_three_phase_and_poe3(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close MTU3 THREE PHASE driver */
    err = R_MTU3_THREE_PHASE_Close(&g_three_phase_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_THREE_PHASE_Close API failed \r\n");
        return err;
    }

    /* Close POE3 driver */
    err = R_POE3_Close(&g_poe3_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_POE3_Close API failed \r\n");
        return err;
    }
    
    return err;
}

/***********************************************************************************************************************
 * Function Name: r_main_mtu_chng_duty
 * Description  : Change the duty rate (25% -> 50% -> 75%) at a repetition.
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void r_main_mtu_chng_duty(void)
{
    fsp_err_t err = FSP_SUCCESS;
    
    g_duty_cnt++;
    
    if (DUTY_CNT_MAX <= g_duty_cnt)
    {
        g_duty_cnt = 0;                      // Clear duty counter 
        
    }

    s_duty_cycle.duty[THREE_PHASE_CHANNEL_U] = g_duty_rate[g_duty_cnt];
    s_duty_cycle.duty[THREE_PHASE_CHANNEL_V] = g_duty_rate[g_duty_cnt];
    s_duty_cycle.duty[THREE_PHASE_CHANNEL_W] = g_duty_rate[g_duty_cnt];
    
    err = R_MTU3_THREE_PHASE_DutyCycleSet(&g_three_phase_ctrl, &s_duty_cycle);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_MTU3_THREE_PHASE_DutyCycleSet API failed \r\n");
        APP_ERR_TRAP (err);
    }
}

/***********************************************************************************************************************
 * Function Name: set_led
 * Description  : Turn on LEDs.
 * Arguments    : LED            select LED to turn on.
 * Return Value : none
 **********************************************************************************************************************/
void set_led(uint8_t LED)
{
    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          // There are no LEDs on this board
        }
    }

    /* Turn on LEDs */
    /* cast to bsp_io_region_t type */
    R_BSP_PinSet((bsp_io_region_t)leds.p_leds[LED][1], (bsp_io_port_pin_t)leds.p_leds[LED][0]);
}

/***********************************************************************************************************************
 * Function Name: clear_led
 * Description  : Turn off all LEDs.
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void clear_led(void)
{
    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          // There are no LEDs on this board
        }
    }

    /* Turn off all LEDs */
    for (uint32_t i = 0; i < leds.led_count; i++)
    {
        /* cast to bsp_io_region_t type */
        R_BSP_PinClear((bsp_io_region_t)leds.p_leds[i][1], (bsp_io_port_pin_t)leds.p_leds[i][0]);
    }

}


/***********************************************************************************************************************
 * @} (end addtogroup r_mtu3_ep)
 **********************************************************************************************************************/
