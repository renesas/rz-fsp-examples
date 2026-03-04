/***********************************************************************************************************************
 * File Name    : adc_ep.h
 * Description  : Contains declarations of data structures and functions used in adc_ep.c
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef ADC_EP_H_
#define ADC_EP_H_

/*******************************************************************************************************************//**
 * @addtogroup r_adc_ep
 * @{
 **********************************************************************************************************************/

/* Macros for Commands to be received through TeraTerm inputs */
#define SCAN_START                      (0x01)
#define SCAN_STOP                       (0x02)

/* Macro for default buff size for reading through TeraTerm */
#define BUFF_SIZE                       (0x0F)

/* Macro to provide delay in read adc data*/
#define ADC_READ_DELAY                  (0x01)

#define SHIFT_BY_ONE                    (0x01)
#define SHIFT_BY_THREE                  (0x03)


/* Macros for menu options to be displayed */
#define MENUOPTION1         "\r\nMENU to Select\r\n"
#define MENUOPTION2         "Press 1 to Start ADC Scan\r\n"
#define MENUOPTION3         "Press 2 to Stop ADC Scan(Only for Continuous mode)\r\n"
#define MENUOPTION4         "User Input :\r\n"

#define EP_INFO             "\r\nThe project initializes the ADC in single scan or continuous scan mode" \
                            "\r\nbased on user selection in RZ configurator.Once initialized,user can start" \
                            "\r\nthe ADC scan and also stop the scan (in the case of continuous scan mode)" \
                            "\r\nusing Terminal Emulator by sending commands.Results are displayed on Terminal Emulator.\r\n"

/* Reading the commands from TeraTerm input and process it */
fsp_err_t read_process_input_from_TeraTerm (void);

/* Read the adc data available */
fsp_err_t adc_read_data (void);

/* close the open adc module  */
void deinit_adc_module (void);

#endif /* ADC_EP_H_ */

/*******************************************************************************************************************//**
 * @} (end addtogroup r_adc_ep)
 **********************************************************************************************************************/
