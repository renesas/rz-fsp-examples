/***********************************************************************************************************************
 * File Name    : dmac_ep.c
 * Description  : Contains data structures and functions used in dmac_ep.c
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "dmac_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_dmac_ep
 * @{
 **********************************************************************************************************************/

/* for on board LEDs */
extern bsp_leds_t       g_bsp_leds;
extern transfer_info_t  g_transfer_info;

/***********************************************************************************************************************
 * @brief     Initialize DMAC driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       DMAC driver opened successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_dmac_driver(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open the transfer instance with initial configuration. */
    err = R_DMAC_Open(&g_transfer_ctrl, &g_transfer_cfg);

    /* Handle any errors.*/
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_DMAC_Open API failed \r\n");
        return err;
    }

    return err;
}

/***********************************************************************************************************************
 * @brief     Initialize CMTW timer driver.
 * @param[IN] None
 * @retval    FSP_SUCCESS       CMTW timer driver opened successfully.
 * @retval    err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_CMTW_timer_driver(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open the CMTW instance. (CMTW timer for DMAC transfer trigger.) */
    err = R_CMTW_Open(&g_timer_ctrl, &g_timer_cfg);

    /* Handle any errors.*/
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_CMTW_Open API failed \r\n");
        return err;
    }

    return err;
}

/***********************************************************************************************************************
 * @brief       Set Transfer settings by using reconfigure API.
 * @param[in]   p_src               Source array               
 * @param[in]   p_dest              Destination array
 * @param[in]   e_mode              DMAC Transfer mode
 * @retval      FSP_SUCCESS         CMTW timer driver opened successfully.
 * @retval      err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
void reconfigure_transfer_settings(uint8_t * p_src, uint8_t * p_dest, transfer_mode_t e_mode)
{

    /* Variable to help handle error codes from functions*/
    fsp_err_t err = FSP_SUCCESS;

    /* get current Transfer setting */
    transfer_info_t transfer_info_recfg = g_transfer_info;

    /* Set new Transfer settings by using reconfigure API */
    if (TRANSFER_MODE_BLOCK == e_mode)
    {
        transfer_info_recfg.p_src  = (void *)(&p_src[0]);
        transfer_info_recfg.p_dest = (void *)(&p_dest[0]);
        transfer_info_recfg.mode   = e_mode;
        transfer_info_recfg.length = TRANSFER_LENGTH_0;

        err = R_DMAC_Reconfigure(&g_transfer_ctrl, &transfer_info_recfg);

    } else if (TRANSFER_MODE_NORMAL == e_mode)
    {
        transfer_info_recfg.p_src  = (void *)(&p_src[0]);
        transfer_info_recfg.p_dest = (void *)(&p_dest[0]);
        transfer_info_recfg.mode   = e_mode;
        transfer_info_recfg.length = TRANSFER_LENGTH_1;

        err = R_DMAC_Reconfigure(&g_transfer_ctrl, &transfer_info_recfg);

    } else
    {
        err = FSP_ERR_UNSUPPORTED;
    }

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* DMAC Reconfigure failed  */
        APP_ERR_PRINT("\r\n ** DMAC Reconfigure FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }
}

/***********************************************************************************************************************
 *  @brief       Print DMAC data transfered by g_transfer_write_ioport on User RTT input
 *  @param[IN]   None
 *  @retval      None
 **********************************************************************************************************************/
void dmac_transfer_print_data(uint8_t *array, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        APP_PRINT("%c", array[i]);

        /* down the line after every 32 characters */
        if (0 == ((i + 1) % 32))
        {
            APP_PRINT("\r\n");
        }
    }
    APP_PRINT("\r\n-------------\n");
}

/***********************************************************************************************************************
 *  @brief        User defined timer callback function.
 *  @param[in]    p_args
 *  @retval       None
 **********************************************************************************************************************/
void timer_callback(timer_callback_args_t * p_args)
{
    if (TIMER_EVENT_CYCLE_END == p_args->event)
    {
        /* Add application code to be called periodically here. */
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

        /* Toggle board LEDs */
        for (uint32_t i = 0; i < leds.led_count; i++)
        {
            R_BSP_PinToggle((bsp_io_region_t)leds.p_leds[i][1], (bsp_io_port_pin_t)leds.p_leds[i][0]);
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_dmac_ep)
 **********************************************************************************************************************/
