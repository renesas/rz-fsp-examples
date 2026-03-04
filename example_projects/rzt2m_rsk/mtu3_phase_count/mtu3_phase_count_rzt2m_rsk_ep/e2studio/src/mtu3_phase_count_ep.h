/***********************************************************************************************************************
 * File Name    : mtu3_phase_count_ep.h
 * Description  : Contains declarations of data structures and functions used in mtu3_phase_count_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef MTU3_PHASE_COUNT_H_
#define MTU3_PHASE_COUNT_H_

/* macro definition */
#define NEW_LINE    ('\n')    /* new line character */
#define NULL_CHAR   ('\0')    /* end of string character */
/* The length of the new count value input */
#define DATA_LENGTH_BYTE        (2)

/* Buffer index for expected end of RTT input array */
#define EXPECTED_CHAR_END_INDEX     (1u)  

#define EP_INFO     "\nThis sample program demonstrates the use of the Multi-Function Timer Pulse Unit 3 (MTU3)" \
                    "\r\nin phase counting mode on the RZ/T2M microcontroller. It counts pulses from" \
                    "\r\na two-phase encoder (A phase, B phase) and provides RTT Viewer-based commands" \
                    "\r\nto get, set, and reset the count value." \
                    "\r\n" \
                    "\r\nOn user input via RTT Viewer:" \
                    "\r\n    \"g\": Retrieves the current count value from the MTU3 counter." \
                    "\r\n    \"s\": Allows the user to set a specific count value." \
                    "\r\n         After entering s, the user inputs the desired value." \
                    "\r\n    \"r\": Resets the MTU3 counter value to zero." \
                    "\r\nAny API/event failure message is also displayed.\r\n\r\n"

#define MENU        "\r\n-------------------------------------------------------------------------------\
                    \r\n[1] Count Get   (g)\
                    \r\n[2] Count Set   (s)\
                    \r\n[3] Count Reset (r)\
                    \r\n-------------------------------------------------------------------------------\n"

/*
 *  Global functions prototype
 */
fsp_err_t init_mtu3_driver (void);

bool input_data_check (char input_data[DATA_LENGTH_BYTE * 2], uint8_t conversionData[DATA_LENGTH_BYTE]);

#endif /* MTU3_PHASE_COUNT_H_ */
