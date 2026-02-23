/***********************************************************************************************************************
 * File Name    : gpt_phase_count_ep.h
 * Description  : Contains declarations of data structures and functions used in gpt_phase_count_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef GPT_PHASE_COUNT_H_
#define GPT_PHASE_COUNT_H_

/* macro definition */

#define NEW_LINE    ('\n')    /* new line character */
#define NULL_CHAR   ('\0')    /* end of string character */
/* The length of the new count value input */
#define DATA_LENGTH_BYTE        (4)

/* Buffer index for expected end of RTT input array */
#define EXPECTED_CHAR_END_INDEX     (1u)  

#define MENU        "\r\n-------------------------------------------------------------------------------\
                    \r\n[1] Count Get   (g)\
                    \r\n[2] Count Set   (s)\
                    \r\n[3] Count Reset (r)\
                    \r\n-------------------------------------------------------------------------------\n"

/*
 *  Global functions prototype
 */
fsp_err_t init_gpt_driver (void);

bool input_data_check (uint8_t input_data[DATA_LENGTH_BYTE * 2], uint8_t conversionData[DATA_LENGTH_BYTE]);

#endif /* GPT_PHASE_COUNT_H_ */
