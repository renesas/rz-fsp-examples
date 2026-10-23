/***********************************************************************************************************************
 * File Name    : ethernet_hpsw_ep.h
 * Description  : Contains declarations of data structures and functions used in ethernet_hpsw_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ETHERNET_HPSW_EP_H
#define ETHERNET_HPSW_EP_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "common_utils.h"
#include "r_hpsw.h"
#include "r_gmac_b.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Pointer of control/ config block for GMAC driver */
#define P_GMAC_CTRL                  (g_ether0.p_ctrl)  /* Pointer of control block for GMAC driver */
#define P_GMAC_CFG                   (g_ether0.p_cfg)   /* Pointer of configuration parameters for GMAC driver */

/***********************************************************************************************************************
 * Public function declarations
 **********************************************************************************************************************/
fsp_err_t initialize_hpsw (hpsw_cfg_t * p_cfg, bool enable);
fsp_err_t transmit_frame (void * tx_buff, uint32_t tx_size);
fsp_err_t deinit_hpsw_module (void);
fsp_err_t deinit_gmac_module (void);
fsp_err_t get_counter_status (hpsw_status_t  * p_hpsw_status,
                              hpsw_counter_t * p_hpsw_counter_A,
                              hpsw_counter_t * p_hpsw_counter_B,
                              hpsw_counter_t * p_hpsw_counter_C);
#endif /* ETHERNET_HPSW_EP_H */
