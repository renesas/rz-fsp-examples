/***********************************************************************************************************************
 * File Name    : ethernet_hpsw_ep.c
 * Description  : Contains data structures and functions used in ethernet_hpsw_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "ethernet_hpsw_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_hpsw_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @brief       This function initializes HPSW module.
 * @param[IN]   p_cfg       Pointer to HPSW configuration.
 * @param[IN]   enable      Enable clock control.
 * @retval      FSP_SUCCESS         Upon successful initialization.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful initialization.
 **********************************************************************************************************************/
fsp_err_t initialize_hpsw(hpsw_cfg_t * p_cfg, bool enable)
{
    fsp_err_t err = FSP_SUCCESS;

    err = R_HPSW_Open(&g_hpsw0_ctrl, &g_hpsw0_cfg);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    /* Adjustable Clock */
    p_cfg->clk_select = HPSW_CLOCK_PTP;
    p_cfg->time_val = 0x1;
    p_cfg->clock_time_adj_value_H = 0x1000;
    p_cfg->clock_time_adj_value_L = 0x2;

    /* PTP TC */
    p_cfg->ptp_tc_profile = HPSW_PTP_TC_DEFAULT;
    p_cfg->ptp_tc_profile_val = 0x1;
    p_cfg->ptp_tc_default_data_1 = 0xff509074;
    p_cfg->ptp_tc_default_data_2 = 0x008877fe;
    p_cfg->ptp_tc_clock_id_val = 0x1;

    /* PTP OC */
    p_cfg->ptp_oc_profile = HPSW_PTP_OC_DEFAULT;
    p_cfg->ptp_oc_dis_offset_correction = 0x1;
    p_cfg->ptp_oc_profile_val = 0x1;
    p_cfg->ptp_oc_default_data_1 = 0xff509074;
    p_cfg->ptp_oc_default_data_2 = 0xddbbaafe;
    p_cfg->ptp_oc_clock_id_val = 0x1;

    /* Set MAC address 06:06:04:50:90:74 */
    p_cfg->mac1 = 0x04509074;
    p_cfg->mac2 = 0x00000606;

    err = R_HPSW_EnableSet(&g_hpsw0_ctrl, p_cfg, enable);

    return err;
}

/***********************************************************************************************************************
 * @brief       This function transmits Ethernet frame.
 * @param[IN]   tx_buff     Transmit buffer.
 * @param[IN]   tx_size     Transmit size.
 * @retval      FSP_SUCCESS     Upon successful transmission.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful transmission.
 **********************************************************************************************************************/
fsp_err_t transmit_frame(void * tx_buff, uint32_t tx_size)
{
    return R_GMAC_B_Write(P_GMAC_CTRL, tx_buff, tx_size);
}

/***********************************************************************************************************************
 *  @brief       This function reads HPSW Port link status and RX/TX counter.
 *  @param[IN]   p_hpsw_status      Pointer to HPSW port link status.
 *  @param[IN]   p_hpsw_counter_A   Pointer to HPSW counter port A.
 *  @param[IN]   p_hpsw_counter_B   Pointer to HPSW counter port B.
 *  @param[IN]   p_hpsw_counter_C   Pointer to HPSW counter port C.
 *  @retval      FSP_SUCCESS        Upon successful read.
 *  @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful read.
 **********************************************************************************************************************/
fsp_err_t get_counter_status(hpsw_status_t  * p_hpsw_status, hpsw_counter_t * p_hpsw_counter_A,
                            hpsw_counter_t * p_hpsw_counter_B, hpsw_counter_t * p_hpsw_counter_C)
{
    fsp_err_t err;
    uint32_t port_A = 0;
    uint32_t port_B = 1;
    uint32_t port_C = 2;

    if ((NULL == p_hpsw_status) || (NULL == p_hpsw_counter_A) ||
        (NULL == p_hpsw_counter_B) || (NULL == p_hpsw_counter_C))
    {
        return FSP_ERR_ASSERTION;
    }

    memset(p_hpsw_status,    0, sizeof(hpsw_status_t));
    memset(p_hpsw_counter_A, 0, sizeof(hpsw_counter_t));
    memset(p_hpsw_counter_B, 0, sizeof(hpsw_counter_t));
    memset(p_hpsw_counter_C, 0, sizeof(hpsw_counter_t));

    err = R_HPSW_StatusGet(&g_hpsw0_ctrl, p_hpsw_status);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_HPSW_CounterGet(&g_hpsw0_ctrl, port_A, p_hpsw_counter_A);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_HPSW_CounterGet(&g_hpsw0_ctrl, port_B, p_hpsw_counter_B);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_HPSW_CounterGet(&g_hpsw0_ctrl, port_C, p_hpsw_counter_C);
    if (FSP_SUCCESS != err)
    {
        return err;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function closes the HPSW driver and handles the return closing API error, to the application.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful close.
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful close.
 **********************************************************************************************************************/
fsp_err_t deinit_hpsw_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close the HPSW driver */
    err = R_HPSW_Close(&g_hpsw0_ctrl);
    return err;
}

/***********************************************************************************************************************
 * @brief       This function closes the GMAC driver and handles the return closing API error, to the application.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful close.
 * @retval      Any Other Error code apart from FSP_SUCCESS  Unsuccessful close.
 **********************************************************************************************************************/
fsp_err_t deinit_gmac_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close the GMAC driver */
    err = R_GMAC_B_Close(P_GMAC_CTRL);
    return err;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_hpsw_ep)
 **********************************************************************************************************************/
