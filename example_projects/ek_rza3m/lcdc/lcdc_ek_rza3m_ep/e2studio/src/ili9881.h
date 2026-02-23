/***********************************************************************************************************************
 * File Name    : ili9881.h
 * Description  : Contains declarations of data structures and functions used in ili9881.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ILI9881_H_
#define ILI9881_H_

#include "r_mipi_dsi_b.h"
#include "bsp_api.h"

/* Simplify and standardize creation of ILI9881 command entries */
#define COMMAND_SEND_DATA(_op, _data0, _data1)\
        ((command_type_t){ .operation = (_op), .data0 = (_data0), .data1 = (_data1) })

typedef struct command_type
{
    uint8_t operation;  /* 0    : Change page to data0 page
                         * 1    : Short packet without parameter
                         * 2    : Short packet with parameter
                         * 0x10 : Delay "data0" milliseconds wait
                         * 0xFF : Terminate command flow */
    uint8_t data0;
    uint8_t data1;
}command_type_t;

extern command_type_t g_command_flow_ili9881[];

/* This function initializes the ILI9881 device. */
void ili9881_init (mipi_dsi_ctrl_t * const p_api_ctrl);

/* This function changes the command page of the ILI9881 device. */
void ili9881_changepage (mipi_dsi_ctrl_t * const p_api_ctrl, uint8_t page);

/* This function sends a command to the ILI9881 device. */
void ili9881_cmd_send (mipi_dsi_ctrl_t * const p_api_ctrl, command_type_t * command_flow);

#endif /* ILI9881_H_ */
