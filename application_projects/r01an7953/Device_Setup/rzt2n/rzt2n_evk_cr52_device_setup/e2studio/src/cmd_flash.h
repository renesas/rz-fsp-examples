/***********************************************************************************************************************
 * File Name    : cmd_flash.h
 * Description  : Contains data structures and functions used in cmd_flash.h.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef CMD_FLASH_H__
#define CMD_FLASH_H__

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
uint8_t cmd_write_flash (uint32_t flash_addr, uint8_t * const p_data, uint32_t data_len);

#endif /* CMD_FLASH_H__ */
