/***********************************************************************************************************************
 * File Name    : adc_ep.c
 * Description  : Contains data structures and functions used in adc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "common_utils.h"
#include "adc_ep.h"

/***********************************************************************************************************************
 * @addtogroup r_adc_ep
 * @{
 **********************************************************************************************************************/

/* Flag to notify that ADC scan is started, so start reading ADC */
volatile bool g_b_ready_to_read = false;

/* Variable to store raw ADC conversion result */
static uint16_t g_adc_data;

/* Flag to indicate that a window comparator event has occurred */
static bool g_window_comp_event = false;

/* Variable to store formatted voltage string for display or logging */
char g_volt_str[5] = {RESET_VALUE};

/* Variable to store converted ADC voltage value */
float g_adc_volt = {RESET_VALUE};

/*
 * private function declarations
 */

/* Open the ADC module, configures and initiates the scan */
static fsp_err_t adc_scan_start (void);

/* Stops the ADC scan if the ADC is continuous scan and then close the module */
static fsp_err_t adc_scan_stop (void);

/***********************************************************************************************************************
 * @brief       This function reads the command (input) from Terminal Emulator and process the command(input).
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful, action of ADC start or ADC stop takes place.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful start or stop.
 **********************************************************************************************************************/
fsp_err_t read_process_input_from_TeraTerm(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    uint8_t readBuff[BUFF_SIZE];
    APP_READ((unsigned char*)readBuff);     /* Cast: readBuff is byte buffer */
    uint32_t rByte = (uint32_t)strlen((char *)readBuff);

    /* Function returns the converted integral number as an INT value */
    int32_t inputRead = atoi((char *)readBuff);

    if (rByte > RESET_VALUE)
    {
        switch (inputRead)
        {
            case SCAN_START:
            {
                err = adc_scan_start();
                break;
            }
            case SCAN_STOP:
            {
                err = adc_scan_stop();
                break;
            }
            default:
            {
                /* Menu for User Selection */
                APP_PRINT(MENUOPTION1);
                APP_PRINT(MENUOPTION2);
                APP_PRINT(MENUOPTION3);
                APP_PRINT(MENUOPTION4);
                break;
            }
        }
    }

    return err;
}

/***********************************************************************************************************************
 * @brief       This function open the ADC, configures and starts the scan.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS               Upon successful open,configure ADC,on success or calibrate and start ADC scan.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful open or configure or start.
 **********************************************************************************************************************/
static fsp_err_t adc_scan_start(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */
    g_window_comp_event = false;

    /* Open/Initialize and start MTU3 Timer module to trigger ADC in repeat mode */
    if (ADC_MODE_CONTINUOUS_SCAN == g_adc_cfg.mode)
    {
        /* Open/Initialize MTU3 module */
        err = R_MTU3_Open(&g_timer0_ctrl, &g_timer0_cfg);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* MTU3 Failure message */
            APP_ERR_PRINT("** R_MTU3_Open API failed **\r\n");
            return err;
        }

        /* Timer start counter */
        err = R_MTU3_Start(&g_timer0_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* MTU3 Failure message */
            APP_ERR_PRINT("** R_MTU3_Start API failed**\r\n");
            return err;
        }
    }

    if (false == g_b_ready_to_read)
    {
        /* Open/Initialize ADC module */
        err = R_ADC_C_Open(&g_adc_ctrl, &g_adc_cfg);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_Open API failed **\r\n");
            return err;
        }

        /* Configures the ADC scan parameters */
        err = R_ADC_C_ScanCfg(&g_adc_ctrl, &g_adc_channel_cfg);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_ScanCfg API failed **\r\n");
            return err;
        }

        /* Start the ADC scan */
        err = R_ADC_C_ScanStart(&g_adc_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_ScanStart API failed **\r\n");
            return err;
        }

        /* Disable interrupts */
        R_BSP_IrqDisable((IRQn_Type)ADC_EVENT_SCAN_COMPLETE);

        APP_PRINT("\r\nADC Started Scan\r\n");

        /* Indication to start reading the ADC data */
        g_b_ready_to_read = true;
    }
    else
    {
        APP_PRINT("\r\nADC Scan already in progress\r\n");
    }

    return err;
}

/***********************************************************************************************************************
 * @brief       This function stops the scanning of ADC.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful stops the ADC scan and closes the ADC.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful stop or close of ADC.
 **********************************************************************************************************************/
static fsp_err_t adc_scan_stop(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Stop the scan if ADC scan is started in continuous scan mode else ignore */
    if ((ADC_MODE_SINGLE_SCAN != g_adc_cfg.mode) && (true == g_b_ready_to_read))
    {
        err = R_ADC_C_ScanStop(&g_adc_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_ScanStop API failed **\r\n");
            return err;
        }

        /* Timer stop */
        err = R_MTU3_Stop(&g_timer0_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_MTU3_Stop API failed **\r\n");
            return err;
        }

        APP_PRINT("\r\nADC Scan stopped\r\n");

        /* Reset to indicate stop reading the ADC data */
        g_b_ready_to_read = false;

        /* Close the ADC module */
        err = R_ADC_C_Close(&g_adc_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_Close API failed **\r\n");
            return err;
        }

        /* Close the timer module */
        err = R_MTU3_Close(&g_timer0_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            APP_ERR_PRINT("** R_MTU3_Close API failed **\r\n");
            return err;
        }
    }
    else
    {
        APP_PRINT("\r\nStop command is not supported in Single Scan mode or User not pressed Start Scan in Repeat mode\r\n");
    }

    APP_PRINT("\r\nPress any other key(except 1 and 2) to go back to the main menu\r\n");
    return err;
}

/***********************************************************************************************************************
 * @brief       This function reads the ADC output data from the prescribed channel and checks ADC status.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS                  Upon successful stops the ADC scan and closes the ADC.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful stop or close of ADC.
 **********************************************************************************************************************/
fsp_err_t adc_read_data(void)
{
    fsp_err_t err = FSP_SUCCESS;     /* Error status */

    /* Read the result */
    err = R_ADC_C_Read(&g_adc_ctrl, ADC_CHANNEL_0, &g_adc_data);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* ADC Failure message */
        APP_ERR_PRINT("** R_ADC_C_Read API failed **\r\n");
        return err;
    }

    g_adc_volt = (float)((g_adc_data * V_REF) / ADC_12_BIT);    /* Cast to float for precision */
    snprintf(g_volt_str, SIZE_64, "%0.2f", g_adc_volt);

    APP_PRINT("\r\nThe Voltage Reading from ADC: %d\r\n", g_adc_data);
    APP_PRINT("\r\nThe ADC input voltage: %s\r\n", g_volt_str);

    /* In ADC single scan mode after reading the data, it stops. So reset the g_b_ready_to_read state to
     * avoid reading unnecessarily. Close the ADC module as it gets opened in start scan command */
    if ((ADC_MODE_SINGLE_SCAN == g_adc_cfg.mode) || (true == g_window_comp_event))
    {
        g_b_ready_to_read = false;

        /* Stop ADC scan */
        err = R_ADC_C_ScanStop(&g_adc_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC ScanStop message */
            APP_ERR_PRINT("** R_ADC_ScanStop API failed **\r\n");
            APP_ERR_TRAP(err);
        }

        /* Close the ADC module */
        err = R_ADC_C_Close(&g_adc_ctrl);

        /* Handle error */
        if (FSP_SUCCESS != err)
        {
            /* ADC Failure message */
            APP_ERR_PRINT("** R_ADC_C_Close API failed **\r\n");
            return err;
        }

        APP_PRINT("\r\nPress any other key(except 1 and 2) to go back to the main menu\r\n");
    }

    /* 1 Seconds Wait time between successive readings */
    R_BSP_SoftwareDelay (ADC_READ_DELAY, BSP_DELAY_UNITS_MILLISECONDS);
    return err;
}

/***********************************************************************************************************************
 * @brief       This function closes the ADC driver and Handle the return closing API error, to the Application.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
void deinit_adc_module(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* Close the ADC driver */
    err = R_ADC_C_Close(&g_adc_ctrl);

    /* Handle error */
    if (FSP_SUCCESS != err)
    {
        /* GPT Close failure message */
        APP_ERR_PRINT("** R_ADC_Close API failed **\r\n");
    }
}

/***********************************************************************************************************************
 * @} (end addtogroup r_adc_ep)
 **********************************************************************************************************************/
