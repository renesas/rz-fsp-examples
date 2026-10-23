/***********************************************************************************************************************
 * File Name    : nandc_ep.c
 * Description  : Contains data structures and functions used in nandc_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "nandc_ep.h"
//#include <stddef.h>
#include <stdint.h>
#include <string.h>

/***********************************************************************************************************************
 * @addtogroup r_nandc_ep
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Global variable definitions
 **********************************************************************************************************************/

/* Command-complete flag updated by the NANDC callback handler */
static volatile uint8_t g_command_complete;

/* Write-protection error flag updated by the NAND callback handler */
static volatile uint8_t g_write_protect_command_error;

/* Most recent NANDC event reported by the callback handler */
static volatile nand_flash_event_t g_callback_event;

/* Most recent NANDC command error status reported by the callback handler */
static volatile nandc_command_status_t g_command_error_status;

/***********************************************************************************************************************
 * Private function declarations
 **********************************************************************************************************************/

/* Wait for the specified NANDC callback event to occur */
static fsp_err_t nandc_wait_for_event (nand_flash_event_t expected_event);

/* Wait for a NANDC write-protection error event */
static fsp_err_t nandc_wait_for_protection_error (void);

/* Reset all callback events */
static void nandc_reset_callback_state (void);

/* Start execution time measurement using the GPT timer */
static fsp_err_t timer_start_measure (void);

/* Read the elapsed execution time from the GPT timer */
static fsp_err_t timer_get_measure (uint32_t * const p_time);

/* Stop the GPT timer measurement */
static fsp_err_t timer_stop (void);

/* Finalize timing measurement */
static fsp_err_t finish_measure (fsp_err_t operation_err, uint32_t * const p_time);

/***********************************************************************************************************************
 * @brief       This function opens the NANDC driver.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_open(void)
{
    return R_NANDC_Open(&g_nand_flash_ctrl, &g_nand_flash_cfg);
}

/***********************************************************************************************************************
 * @brief       This function initializes the GPT timer used for NANDC operation measurement.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_timer_init(void)
{
    return R_GPT_Open(&g_timer_ctrl, &g_timer_cfg);
}

/***********************************************************************************************************************
 * @brief       This function erases one NAND block and returns the execution time.
 * @param[IN]   block Block number to erase.
 * @param[OUT]  p_time Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_erase_block(uint32_t block, uint32_t * const p_time)
{
    fsp_err_t err;
    if (NULL == p_time) 
    { 
        return FSP_ERR_INVALID_ARGUMENT; 
    }
    nandc_reset_callback_state();

    /* Start time measurement */
    err = timer_start_measure();
    if (FSP_SUCCESS != err) { return err; }

    err = R_NANDC_Erase(&g_nand_flash_ctrl, block, 1U, NAND_FLASH_CHIP_SELECT);
    if (FSP_SUCCESS == err) 
    {
        /* Wait till operation completes */
        err = nandc_wait_for_event(NAND_FLASH_EVENT_ERASE_COMPLETE);
    }

    /* Get execution time */
    return finish_measure(err, p_time);
}

/***********************************************************************************************************************
 * @brief       This function writes one or more normal NAND pages and returns the execution time.
 * @param[IN]   p_main_source       Pointer to the main-data source buffer.
 * @param[IN]   p_oob_source        Pointer to the optional OOB source buffer.
 * @param[IN]   row_address         First row address to write.
 * @param[IN]   page_count          Number of pages to write.
 * @param[OUT]  p_time Pointer      to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS         Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_write_pages(uint8_t const * p_main_source,
                            uint8_t const * p_oob_source,
                            uint32_t row_address,
                            uint32_t page_count,
                            uint32_t * const p_time)
{
    fsp_err_t err;
    nand_flash_device_page_t page = 
    {
        .row_address = row_address,
        .page_count = page_count
    };
    nand_flash_data_buffer_t data = 
    {
        .p_main_data_buffer = (uint8_t *) p_main_source,
        .p_oob_data_buffer = (uint8_t *) p_oob_source
    };
    if ((NULL == p_main_source) || (NULL == p_time) || (0U == page_count))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    nandc_reset_callback_state();

    /* Start time measurement */
    err = timer_start_measure();
    if (FSP_SUCCESS != err) { return err; }

    err = R_NANDC_Write(&g_nand_flash_ctrl, &data, &page, NAND_FLASH_CHIP_SELECT);
    if (FSP_SUCCESS == err)
    {
        /* Wait till operation completes */
        err = nandc_wait_for_event(NAND_FLASH_EVENT_WRITE_COMPLETE);
    }

    /* Get execution time */
    return finish_measure(err, p_time);
}

/***********************************************************************************************************************
 * @brief       This function reads one or more normal NAND pages and returns the execution time.
 * @param[IN]   row_address         First row address to read.
 * @param[IN]   page_count          Number of pages to read.
 * @param[OUT]  p_main_destination  Pointer to the main-data destination buffer.
 * @param[OUT]  p_oob_destination   Pointer to the optional OOB destination buffer.
 * @param[OUT]  p_time              Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS         Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_read_pages(uint32_t row_address,
                           uint32_t page_count,
                           uint8_t * p_main_destination,
                           uint8_t * p_oob_destination,
                           uint32_t * const p_time)
{
    fsp_err_t err;
    nand_flash_device_page_t page =
    {
        .row_address = row_address,
        .page_count = page_count
    };
    nand_flash_data_buffer_t data =
    {
        .p_main_data_buffer = p_main_destination,
        .p_oob_data_buffer = p_oob_destination
    };
    if ((NULL == p_main_destination) || (NULL == p_time) || (0U == page_count))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    nandc_reset_callback_state();

    /* Start time measurement */
    err = timer_start_measure();
    if (FSP_SUCCESS != err) { return err; }

    err = R_NANDC_Read(&g_nand_flash_ctrl, &page, &data, NAND_FLASH_CHIP_SELECT);
    if (FSP_SUCCESS == err)
    {
        /* Wait till operation completes */
        err = nandc_wait_for_event(NAND_FLASH_EVENT_READ_COMPLETE);
    }

    /* Get execution time */
    return finish_measure(err, p_time);
}

/***********************************************************************************************************************
 * @brief       This function writes one or more raw physical NAND pages and returns the execution time.
 * @param[IN]   p_source        Pointer to the raw source buffer.
 * @param[IN]   row_address     First row address to write.
 * @param[IN]   page_count      Number of pages to write.
 * @param[OUT]  p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_raw_write_pages(uint8_t const * p_source,
                                uint32_t row_address,
                                uint32_t page_count,
                                uint32_t * const p_time)
{
    fsp_err_t err;
    nand_flash_device_page_t page =
    {
        .row_address = row_address,
        .page_count = page_count
    };
    if ((NULL == p_source) || (NULL == p_time) || (0U == page_count))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Wait for the controller to become ready before operation */
    err = nandc_wait_controller_ready();
    if (FSP_SUCCESS != err) { return err; }

    nandc_reset_callback_state();

    /* Start time measurement */
    err = timer_start_measure();
    if (FSP_SUCCESS != err) { return err; }

    err = R_NANDC_RawWrite(&g_nand_flash_ctrl, p_source, &page, NAND_FLASH_CHIP_SELECT);
    if (FSP_SUCCESS == err)
    {
        /* Wait till operation completes */
        err = nandc_wait_for_event(NAND_FLASH_EVENT_WRITE_COMPLETE);
    }

    /* Get execution time */
    return finish_measure(err, p_time);
}

/***********************************************************************************************************************
 * @brief       This function reads one or more raw physical NAND pages and returns the execution time.
 * @param[IN]   row_address     First row address to read.
 * @param[IN]   page_count      Number of pages to read.
 * @param[OUT]  p_destination   Pointer to the raw destination buffer.
 * @param[OUT]  p_time          Pointer to the execution time in nanoseconds.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_raw_read_pages(uint32_t row_address,
                               uint32_t page_count,
                               uint8_t * p_destination,
                               uint32_t * const p_time)
{
    fsp_err_t err;
    nand_flash_device_page_t page =
    {
        .row_address = row_address,
        .page_count = page_count
    };
    if ((NULL == p_destination) || (NULL == p_time) || (0U == page_count))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Wait for the controller to become ready before operation */
    err = nandc_wait_controller_ready();
    if (FSP_SUCCESS != err) { return err; }
 
    nandc_reset_callback_state();

    /* Start time measurement */
    err = timer_start_measure();
    if (FSP_SUCCESS != err) { return err; }

    err = R_NANDC_RawRead(&g_nand_flash_ctrl, &page, p_destination, NAND_FLASH_CHIP_SELECT);
    if (FSP_SUCCESS == err)
    {
        /* Wait till operation completes */
        err = nandc_wait_for_event(NAND_FLASH_EVENT_READ_COMPLETE);
    }

    /* Get execution time */
    return finish_measure(err, p_time);
}

/***********************************************************************************************************************
 * @brief       This function configures one logical-to-physical remap entry.
 * @param[IN]   logical_address     Logical row address.
 * @param[IN]   physical_address    Physical row address.
 * @retval      FSP_SUCCESS         Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_remap_set(uint32_t logical_address, uint32_t physical_address)
{
    return R_NANDC_RemapSet(&g_nand_flash_ctrl, logical_address, physical_address);
}

/***********************************************************************************************************************
 * @brief       This function clears the NANDC remap table.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_remap_clear(void)
{
    return R_NANDC_RemapClear(&g_nand_flash_ctrl);
}

/***********************************************************************************************************************
 * @brief       This function enables a NANDC write-protection range.
 * @param[IN]   start_address   Start row address of the protected range.
 * @param[IN]   size            Size of the protected range.
 * @param[IN]   index           Protection-range index.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_write_protect_enable(uint32_t start_address, uint32_t size, uint32_t index)
{
    return R_NANDC_WriteProtectEnable(&g_nand_flash_ctrl, start_address, size, index);
}

/***********************************************************************************************************************
 * @brief       This function disables a NANDC write-protection range.
 * @param[IN]   index           Protection-range index.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_write_protect_disable(uint32_t index)
{
    return R_NANDC_WriteProtectDisable(&g_nand_flash_ctrl, index);
}

/***********************************************************************************************************************
 * @brief       This function verifies that a protected write is rejected.
 * @param[IN]   p_main_source    Pointer to the main-data source buffer.
 * @param[IN]   p_oob_source     Pointer to the optional OOB source buffer.
 * @param[IN]   row_address      First row address to write.
 * @param[IN]   page_count       Number of pages to write.
 * @retval      FSP_SUCCESS      Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_write_protected_pages(uint8_t const * p_main_source,
                                      uint8_t const * p_oob_source,
                                      uint32_t row_address,
                                      uint32_t page_count)
{
    fsp_err_t err;
    nand_flash_device_page_t page =
    {
        .row_address = row_address,
        .page_count = page_count
    };
    nand_flash_data_buffer_t data =
    {
        .p_main_data_buffer = (uint8_t *) p_main_source,
        .p_oob_data_buffer = (uint8_t *) p_oob_source
    };
    if ((NULL == p_main_source) || (0U == page_count))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    nandc_reset_callback_state();

    err = R_NANDC_Write(&g_nand_flash_ctrl, &data, &page, NAND_FLASH_CHIP_SELECT);
    return (FSP_SUCCESS == err) ? nandc_wait_for_protection_error() : err;
}

/***********************************************************************************************************************
 * @brief       This function waits until the NANDC controller is ready.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS      Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_wait_controller_ready(void)
{
    fsp_err_t err;
    nandc_controller_status_t status;
    uint32_t wait_count = NANDC_CALLBACK_WAIT_LIMIT;
    do
    {
        err = R_NANDC_ControllerStatusGet(&g_nand_flash_ctrl, &status);
        if (FSP_SUCCESS != err) { return err; }
        if (NANDC_CONTROLLER_STATE_BUSY != status.status) 
        {
            return FSP_SUCCESS;
        }
        wait_count--;
    } while (0U != wait_count);
    return FSP_ERR_TIMEOUT;
}

/***********************************************************************************************************************
 * @brief       This function compares two buffers.
 * @param[IN]   p_expected      Pointer to expected data.
 * @param[IN]   p_actual        Pointer to actual data.
 * @param[IN]   length          Number of bytes to compare.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS  Unsuccessful operation.
 **********************************************************************************************************************/
fsp_err_t nandc_verify_buffer(uint8_t const * p_expected, uint8_t const * p_actual, uint32_t length)
{
    if ((NULL == p_expected) || (NULL == p_actual) || (0U == length))
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    return (0 == memcmp(p_expected, p_actual, length)) ? FSP_SUCCESS : FSP_ERR_INVALID_STATE;
}

/***********************************************************************************************************************
 * @brief       This function fills a buffer with a constant pattern.
 * @param[OUT]  p_buffer     Pointer to the destination buffer.
 * @param[IN]   length       Number of bytes to fill.
 * @param[IN]   pattern      Pattern value.
 * @retval      None.
 **********************************************************************************************************************/
void nandc_fill_constant_pattern(uint8_t * p_buffer, uint32_t length, uint8_t pattern)
{
    if (NULL != p_buffer)
    {
        memset(p_buffer, pattern, length);
    }
}

/***********************************************************************************************************************
 * @brief       This function handles NANDC callback events.
 * @param[IN]   p_args    Pointer to callback arguments.
 * @retval      None.
 **********************************************************************************************************************/
void nandc_example_callback(nand_flash_callback_args_t * p_args)
{
    g_callback_event = p_args->event;
    if ((NAND_FLASH_EVENT_ERASE_COMPLETE == p_args->event) ||
        (NAND_FLASH_EVENT_READ_COMPLETE  == p_args->event) ||
        (NAND_FLASH_EVENT_WRITE_COMPLETE == p_args->event))
    {
        g_command_complete = 1U;
    }
    if (NAND_FLASH_EVENT_WRITE_ERROR == p_args->event)
    {
        g_write_protect_command_error = 1U;
        g_command_error_status.command_status = p_args->error;
    }
}

/***********************************************************************************************************************
 * @brief       This function waits for an expected NANDC callback event.
 * @param[IN]   expected_event   Expected callback event.
 * @retval      FSP_SUCCESS      Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_wait_for_event(nand_flash_event_t expected_event)
{
    uint32_t wait_count = NANDC_CALLBACK_WAIT_LIMIT;
    while ((0U == g_command_complete) && (0U != wait_count))
    {
        wait_count--;
    }
    if (0U == g_command_complete) { return FSP_ERR_TIMEOUT; }
    return (expected_event == g_callback_event) ? FSP_SUCCESS : FSP_ERR_INVALID_STATE;
}

/***********************************************************************************************************************
 * @brief       This function waits for and verifies a protected-write error.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t nandc_wait_for_protection_error(void)
{
    uint32_t wait_count = NANDC_CALLBACK_WAIT_LIMIT;
    while ((0U == g_write_protect_command_error) && (0U != wait_count))
    {
        wait_count--;
    }
    if (0U == g_write_protect_command_error)
    {
        return FSP_ERR_TIMEOUT;
    }
    return (0U != g_command_error_status.nandc_command_status_bit.prot_err) ? FSP_SUCCESS : FSP_ERR_INVALID_STATE;
}

/***********************************************************************************************************************
 * @brief       This function resets NANDC callback states.
 * @param[IN]   None.
 * @retval      None.
 **********************************************************************************************************************/
static void nandc_reset_callback_state(void)
{
    FSP_CRITICAL_SECTION_DEFINE;
    FSP_CRITICAL_SECTION_ENTER;
    g_command_complete = 0U;
    g_write_protect_command_error = 0U;
    g_callback_event = (nand_flash_event_t) 0;
    g_command_error_status.command_status = 0U;
    FSP_CRITICAL_SECTION_EXIT;
}

/***********************************************************************************************************************
 * @brief       This function starts GPT measurement.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS      Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t timer_start_measure(void)
{
    /* Clear timer counter */
    fsp_err_t err = R_GPT_Reset(&g_timer_ctrl);

    /* Start timer */
    return (FSP_SUCCESS == err) ? R_GPT_Start(&g_timer_ctrl) : err;
}

/***********************************************************************************************************************
 * @brief       This function gets GPT measurement in nanoseconds and stops GPT.
 * @param[OUT]  p_time          Pointer to measured time.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS     Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t timer_get_measure(uint32_t * const p_time)
{
    fsp_err_t err;
    timer_status_t status = {0};
    timer_info_t info =
    {
        .count_direction = TIMER_DIRECTION_UP,
        .clock_frequency = 0U,
        .period_counts   = 0U
    };
    if (NULL == p_time)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Get status of timer */
    err = R_GPT_StatusGet(&g_timer_ctrl, &status);
    if (FSP_SUCCESS == err)
    {
        err = R_GPT_InfoGet(&g_timer_ctrl, &info);
    }
    if (FSP_SUCCESS == err)
    {
        /* Stop timer */
        err = R_GPT_Stop(&g_timer_ctrl);
    }
    if (FSP_SUCCESS != err)
    {
        return err;
    }
    if (0U == info.clock_frequency)
    {
        return FSP_ERR_INVALID_STATE;
    }

    /* Convert count value to nanoseconds unit */
    /* (counter * 100) / (clock_frequency / 10000000) */
    *p_time = (uint32_t) ((((uint64_t) status.counter) * 1000000000ULL) / info.clock_frequency);
    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * @brief       This function stops GPT measurement.
 * @param[IN]   None.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t timer_stop(void)
{
    return R_GPT_Stop(&g_timer_ctrl);
}

/***********************************************************************************************************************
 * @brief       This function finalizes GPT measurement while preserving the operation result.
 * @param[IN]   operation_err   NANDC operation result.
 * @param[OUT]  p_time          Pointer to measured time.
 * @retval      FSP_SUCCESS     Upon successful operation.
 * @retval      Any other error code apart from FSP_SUCCESS      Unsuccessful operation.
 **********************************************************************************************************************/
static fsp_err_t finish_measure(fsp_err_t operation_err, uint32_t * const p_time)
{
    fsp_err_t timer_err;
    if (FSP_SUCCESS != operation_err)
    {
        timer_err = timer_stop();
        return (FSP_SUCCESS == timer_err) ? operation_err : timer_err;
    }

    /* Get GPT timer measurement in nanoseconds */
    timer_err = timer_get_measure(p_time);
    return (FSP_SUCCESS == timer_err) ? operation_err : timer_err;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_nandc_ep)
 **********************************************************************************************************************/
