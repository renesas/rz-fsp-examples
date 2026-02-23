/***********************************************************************************************************************
 * File Name    : ili9881.c
 * Description  : Contains data structures and functions used in ili9881.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "ili9881.h"
#include "hal_data.h"
#include "r_mipi_dsi_b.h"

extern volatile bool g_message_sent;

/***********************************************************************************************************************
 *  @brief      This function initializes the ILI9881 device.
 *  @param[in]  None.
 *  @retval     None.
 **********************************************************************************************************************/
void ili9881_init(mipi_dsi_ctrl_t * const p_api_ctrl)
{
    ili9881_cmd_send(p_api_ctrl, g_command_flow_ili9881);
}

/***********************************************************************************************************************
 * @brief      This function changes the command page of the ILI9881 device.
 * @param[in]  p_api_ctrl    Pointer to the MIPI DSI control block.
 * @param[in]  page          Page number to switch to.
 * @retval     None.
 **********************************************************************************************************************/
void ili9881_changepage(mipi_dsi_ctrl_t * const p_api_ctrl, uint8_t page)
{
    uint8_t send_data[4];

    send_data[0] = 0xFF;
    send_data[1] = 0x98;
    send_data[2] = 0x81;
    send_data[3] = page;

    mipi_dsi_cmd_t cmd =
    {
        .channel = 0,
        .cmd_id =(mipi_cmd_id_t) MIPI_DSI_CMD_ID_DCS_LONG_WRITE,
        .flags = (mipi_dsi_cmd_flag_t) (MIPI_DSI_CMD_FLAG_LOW_POWER),   /* Cast to mipi_dsi_cmd_flag_t */
        .tx_len = 4,
        .p_tx_buffer = send_data,
    };

    g_message_sent = false;
    R_MIPI_DSI_B_Command(p_api_ctrl, &cmd);
    while (!g_message_sent)
    {
        ;   /* Do nothing */
    }
}

/***********************************************************************************************************************
 * @brief      This function sends a command to the ILI9881 device.
 * @param[in]  p_api_ctrl      Pointer to the MIPI DSI control block.
 * @param[in]  command_flow    Pointer to an array of command_type_t structures.
 * @retval     None.
 **********************************************************************************************************************/
void ili9881_cmd_send(mipi_dsi_ctrl_t * const p_api_ctrl, command_type_t * command_flow)
{
    uint32_t i = 0;

    while (0xFF != command_flow[i].operation)
    {
        if (0 == command_flow[i].operation)
        {
            ili9881_changepage(p_api_ctrl, command_flow[i].data0);
        }
        else if (1 == command_flow[i].operation)
        {
            uint8_t send_data[2];
            send_data[0] = command_flow[i].data0;
            send_data[1] = command_flow[i].data1;
            mipi_dsi_cmd_t cmd =
            {
                .channel = 0,
                .cmd_id = (mipi_cmd_id_t) MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_0_PARAM,
                .flags = (mipi_dsi_cmd_flag_t) (MIPI_DSI_CMD_FLAG_LOW_POWER),   /* Cast to mipi_dsi_cmd_flag_t */
                .tx_len = 2,
                .p_tx_buffer = send_data,
            };

            g_message_sent = false;
            R_MIPI_DSI_B_Command(p_api_ctrl, &cmd);
            while (!g_message_sent)
            {
                ;   /* Do nothing */
            }
        }
        else if (2 == command_flow[i].operation)
        {
            uint8_t send_data[2];
            send_data[0] = command_flow[i].data0;
            send_data[1] = command_flow[i].data1;
            mipi_dsi_cmd_t cmd =
            {
                .channel = 0,
                .cmd_id = (mipi_cmd_id_t) MIPI_DSI_CMD_ID_DCS_SHORT_WRITE_1_PARAM,
                .flags = (mipi_dsi_cmd_flag_t) (MIPI_DSI_CMD_FLAG_LOW_POWER),   /* Cast to mipi_dsi_cmd_flag_t */
                .tx_len = 2,
                .p_tx_buffer = send_data,
            };

            g_message_sent = false;
            R_MIPI_DSI_B_Command(p_api_ctrl, &cmd);
            while (!g_message_sent)
            {
                ;   /* Do nothing */
            }
        }
        else if (0x10 == command_flow[i].operation)
        {
            R_BSP_SoftwareDelay(command_flow[i].data0, BSP_DELAY_UNITS_MILLISECONDS);
        }
        else
        {
            ;   /* No operation */
        }
        i++;
    }
}
