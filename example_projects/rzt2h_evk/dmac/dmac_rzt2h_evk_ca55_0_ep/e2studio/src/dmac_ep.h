/***********************************************************************************************************************
 * File Name    : dmac_ep.h
 * Description  : Contains data structures and functions declarations used in dmac_ep.c
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef DMAC_EP_H_
#define DMAC_EP_H_

/* array size for DMAC transfer data */
#define TRANSFER_LENGTH_0 (512)
#define TRANSFER_LENGTH_1 (64)


#define EP_INFO     "\r\nThis example project demonstrates a typical use case of the DMAC module." \
                    "\r\nIt initializes the DMAC functionality and performs data transfers from" \
                    "\r\na source array located in ATCM to a destination array." \
                    "\r\nThe transfers are triggered using both a software trigger and a CMTW" \
                    "\r\ncompare match event." \
                    "\r\nUpon successful completion, the received data is displayed on the Tera Term." \
                    "\r\nAny API/event failure message is also displayed.\r\n\r\n"

/*
 *  Global functions
 */
fsp_err_t init_dmac_driver (void);
fsp_err_t init_CMTW_timer_driver (void);

void reconfigure_transfer_settings (uint8_t * p_src, uint8_t * p_dest, transfer_mode_t e_mode);
void dmac_transfer_print_data (uint8_t * array, uint32_t length);

#endif /* DMAC_EP_H_ */
