/***********************************************************************************************************************
 * File Name    : transfer_initialise.h
 * Description  : Contains declarations of data structures and functions used in transfer_initialise.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef TRANSFER_INITIALISE_H_
#define TRANSFER_INITIALISE_H_

#define SOURCE_DATA_SIZE            (60U)          /* Macro for g_transfer_led_blink transfer array size */
#define DEST_DATA_SIZE              (8U)           /* Macro for destination array size */
#define DATA_SIZE                   (8U)           /* Macro for data size */
#define EXPECTED_CHAR_END_INDEX     (1u)           /* Buffer index for expected end of Tera Term input array */

/* Macros for Tera Term input to software start DMAC g_transfer_mtu3_value transfer */
#define START_TRANSFER_ON_LED_PORT  (1u)
#define START_TRANSFER_MTU_VALUE    (2u)

/* Source and destination array for DMAC transfer */
extern uint8_t g_source_data[SOURCE_DATA_SIZE];
extern uint32_t g_dest_data[DATA_SIZE];

/* This function initializes the transfer unit module */
fsp_err_t dmac_transfer_init (dmac_instance_ctrl_t * const g_transfer_ctl,
                              transfer_cfg_t const * const g_transfer_cfg);

/* This function de-initializes the transfer unit module */
void dmac_transfer_deinit (dmac_instance_ctrl_t * const p_transfer_ctl);

/* This function performs software start for DMAC transfer instance */
fsp_err_t dmac_transfer_software_start (transfer_ctrl_t * const p_transfer_ctrl);

/* This function prints the data transferred by g_transfer_mtu_value */
void dmac_transfer_print_data (void);

/* This function sets the source and destination addresses */
fsp_err_t set_transfer_dst_src_address (transfer_cfg_t const * const p_config,
                                        void const * volatile p_src,
                                        void const * volatile p_dest,
                                        mmu_ctrl_t * const p_mmu_ctrl);

#endif /* TRANSFER_INITIALISE_H_ */
