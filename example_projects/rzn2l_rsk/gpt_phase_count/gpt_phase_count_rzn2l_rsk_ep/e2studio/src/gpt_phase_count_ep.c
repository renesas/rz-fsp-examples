/***********************************************************************************************************************
 * File Name    : gpt_phase_count_ep.c
 * Description  : Contains data structures and functions used in gpt_phase_count_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "hal_data.h"
#include "common_utils.h"
#include "gpt_phase_count_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_gpt_ep
 * @{
 **********************************************************************************************************************/
/*
 *  local functions prototype
 */
static void convert_char_to_hex (char * input, uint8_t * result);

/***********************************************************************************************************************
 * @brief     Initialize GPT driver and start Phase Count.
 * @param[IN] None
 * @retval    FSP_SUCCESS       GPT driver opened successfully and start Phase Count.
 * @retval    err               Any Other Error code apart from FSP_SUCCES like Unsuccessful Open.
 **********************************************************************************************************************/
fsp_err_t init_gpt_driver(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Open GPT driver */
    err = R_GPT_Open(&g_timer_ctrl, &g_timer_cfg);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_GPT_Open API failed \r\n");
        return err;
    }

    /* GPT Phase Count start */
    err = R_GPT_Start(&g_timer_ctrl);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT ("R_GPT_Start API failed \r\n");
        return err;
    }
    return err;
}

/***********************************************************************************************************************
 * Function Name: input_data_check
 * Description  : Check that the input address follows the format below.
 *              : The input value consists only of the characters "0~9, a~f, A~F".
 *              : It also converts the data into a type that can be transmitted after confirmation.
 *              : Note. This function is applied to addresses and data entered by the user.
 * Arguments    : char input_data[DATA_LENGTH_BYTE * 2]    ; I : Input address
 *              : uint8_t conversionData[DATA_LENGTH_BYTE] ; O : Converted address
 * Return Value : true  : Input address is correct
 *              : false : Input address is incorrect
 **********************************************************************************************************************/
bool input_data_check(char input_data[DATA_LENGTH_BYTE * 2], uint8_t conversionData[DATA_LENGTH_BYTE])
{
    int32_t check_ascii = 0;

    size_t size;

    char conv_data[DATA_LENGTH_BYTE * 2];

    /* As initialization, fill the value of '0' */
    memset(conv_data, 0x30, sizeof(conv_data));
    
    /* Convert data if less than 8 digits */
    size = strlen(input_data);
    if (size < (DATA_LENGTH_BYTE * 2))
    {
        for (int32_t cnt = 0; cnt < (int32_t) size; cnt++)  /* cast to int32_t type */
        {
            conv_data[(DATA_LENGTH_BYTE * 2) - ((int32_t) size - cnt)] = input_data[cnt];   /* cast to int32_t type */
        }
        memcpy(input_data, conv_data, sizeof(conv_data));
    }

    /* Check input values */
    for (int32_t i = 0; i < (DATA_LENGTH_BYTE * 2); i++)
    {
        check_ascii = input_data[i];

        if (!((('0' <= check_ascii) && ('9' >= check_ascii)) || (('a' <= check_ascii) && ('f' >= check_ascii)) ||
                (('A' <= check_ascii) && ('F' >= check_ascii))))
        {
            APP_PRINT("\r\n[ERR] Incorrect characters are used.\n");
            return false;
        }
    }

    /* Conversion to uint8_t array */
    convert_char_to_hex(input_data, conversionData);

    return true;
}

/***********************************************************************************************************************
 * Function Name: convert_char_to_hex
 * Description  : Converts an char array to a hexadecimal array.
 * Arguments    : char *input     ; I : Char array to be converted.
 *              : uint8_t *result ; O : Conversion Result.
 * Return Value : none
 **********************************************************************************************************************/
static void convert_char_to_hex(char *input, uint8_t *result)
{
    uint32_t input_value_array[2];

    uint32_t byte_data      = 0;
    uint32_t array_pos      = 0;
    uint32_t result_count   = 0;

    for (uint8_t n = 0; n < (DATA_LENGTH_BYTE * 2); n++)
    {
        switch (input[n])
        {
            case 'a':
            case 'A':
                input_value_array[array_pos] = 10;
                break;
            case 'b':
            case 'B':
                input_value_array[array_pos] = 11;
                break;
            case 'c':
            case 'C':
                input_value_array[array_pos] = 12;
                break;
            case 'd':
            case 'D':
                input_value_array[array_pos] = 13;
                break;
            case 'e':
            case 'E':
                input_value_array[array_pos] = 14;
                break;
            case 'f':
            case 'F':
                input_value_array[array_pos] = 15;
                break;
            default:
                input_value_array[array_pos] = (uint32_t)(input[n] - '0'); /* cast to uint32_t type */
                break;
        }

        if (array_pos >= 1)
        {
            byte_data               = (input_value_array[0] * 16) + input_value_array[1];
            result[result_count]    = (uint8_t)byte_data; /* cast to uint8_t type */
            array_pos               = 0;
            result_count++;
        }
        else
        {
            array_pos++;
        }
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_gpt_ep)
 **********************************************************************************************************************/
