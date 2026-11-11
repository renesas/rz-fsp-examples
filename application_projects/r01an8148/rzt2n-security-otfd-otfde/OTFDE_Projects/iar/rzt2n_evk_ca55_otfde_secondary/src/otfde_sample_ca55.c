/***********************************************************************************************************************
 * File Name    : otfde_sample_ca55.c
 * Description  : Contains data structures and functions used in otfde_sample_ca55.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "hal_data.h"
#include "otfde_sample.h"
#include <stdio.h>

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define GPT_CLOCK_HZ                      (400000000U)  /* RZ/T2N GPT counter frequency */
#define GPT_MAX_COUNTS                    (0xFFFFFFFFU) /* GPT count number when wrapping around */

/* DDR of OTFDE enabled area. */
#define DDR_DATA_SINGLE_CORE_ADDR         ((uint64_t)0xE0001000UL)
#define DDR_DATA_CROSS_CORE_ADDR          ((uint64_t)0xE0011000UL)

/* In order to output the verification results in CR52, store the CA55 verification results in the following area. */
#define READ_DATA_CA55_SINGLE_CORE_ADDR   ((uint64_t)0x103E0000UL)
#define READ_DATA_CA55_CROSS_CORE_ADDR    ((uint64_t)0x103F0000UL)

#define WRITE_TIME_CA55_CPU_ADDR          ((uint64_t)0x103F0100UL)
#define WRITE_TIME_CA55_DMAC_ADDR         ((uint64_t)0x103F0200UL)
#define READ_TIME_CA55_CPU_ADDR           ((uint64_t)0x103F0300UL)
#define READ_TIME_CA55_DMAC_ADDR          ((uint64_t)0x103F0400UL)

#define COMPARE_RESULT_CA55_ADDR          ((uint64_t)0x103F0500UL)

typedef enum e_compare_result
{
    COMPARE_RESULT_MATCH      = 0x00U,
    COMPARE_RESULT_UNMATCH    = 0x01U,
} compare_result_t;

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static uint32_t gs_perf_gpt_wraps_cnt               = 0;

/* DDR write destination area. */
static uint8_t * gsp_ddr_data_single_core           = (uint8_t *)DDR_DATA_SINGLE_CORE_ADDR;
static uint8_t * gsp_ddr_data_cross_core            = (uint8_t *)DDR_DATA_CROSS_CORE_ADDR;

/* Variables for storing results for CA55. */
static uint8_t * gsp_read_data_ca55_single_core     = (uint8_t *)READ_DATA_CA55_SINGLE_CORE_ADDR;
static uint8_t * gsp_read_data_ca55_cross_core      = (uint8_t *)READ_DATA_CA55_CROSS_CORE_ADDR;

static double * gsp_write_time_ca55_cpu             = (double *)WRITE_TIME_CA55_CPU_ADDR;
static double * gsp_write_time_ca55_dmac            = (double *)WRITE_TIME_CA55_DMAC_ADDR;
static double * gsp_read_time_ca55_cpu              = (double *)READ_TIME_CA55_CPU_ADDR;
static double * gsp_read_time_ca55_dmac             = (double *)READ_TIME_CA55_DMAC_ADDR;
static compare_result_t * gsp_compare_result_ca55   = (compare_result_t *)COMPARE_RESULT_CA55_ADDR;

static volatile bool gs_intcpu1_notification_receive_flag = false;
static volatile bool gs_dmac_transfer_end_flag            = false;

static void otfde_writeread_verify_ca55_cpu (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                        uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time);
static void otfde_writeread_verify_ca55_dmac (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                        uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time);
static void otfde_writeread_verify_cross_core (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                                uint8_t * p_read_data_from_ddr, compare_result_t * p_compare_result);
static void set_dmac_transfer (uint8_t * p_dest, uint8_t * p_src, uint32_t data_len);
static double perf_calc_elapsed_time (uint32_t counter);
static void handle_error (fsp_err_t err);

/******************************************************************************
 * @fn       otfde_sample
 * @brief    OTFDE operation verification process.
 ******************************************************************************/
void otfde_sample(void)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    
    /* Initialize each module and enable interrupts. */
    __enable_irq();
    
    err = R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);
    handle_error(err);
    
    err = R_DMAC_Open(&g_transfer1_ctrl, &g_transfer1_cfg);
    handle_error(err);
    
    /* Use INTCPU0 to generate a software interrupt from CA55 to CR52. */
    err = R_ICU_INTER_CPU_IRQ_Open(&g_icu_inter_cpu_irq0_ctrl, &g_icu_inter_cpu_irq0_cfg);
    handle_error(err);
    
    /* Use INTCPU1 to generate a software interrupt from CR52 to CA55. */
    err = R_ICU_INTER_CPU_IRQ_Open(&g_icu_inter_cpu_irq1_ctrl, &g_icu_inter_cpu_irq1_cfg);
    handle_error(err);
    
    /* ------------------------------------------------------------- */
    /* The secondary side notifies the primary side that execution has begun. */
    /* Generates an interrupt(INTCPU0) to CR52. */
    err = R_ICU_INTER_CPU_IRQ_Generate(&g_icu_inter_cpu_irq0_ctrl);
    handle_error(err);
    /* ------------------------------------------------------------- */
    
    /* Waiting for an interrupt(INTCPU1) from CR52. */
    while (false == gs_intcpu1_notification_receive_flag);
    gs_intcpu1_notification_receive_flag = false;
    
    /* Verify that CA55 writes and reads to DDR of OTFDE enabled area with CPU transfer. */
    /* Performance is measured during this time. */
    otfde_writeread_verify_ca55_cpu(gsp_ddr_data_single_core, g_plaintext_single_core, SINGLE_CORE_DATA_LEN,
                                    gsp_read_data_ca55_single_core, gsp_write_time_ca55_cpu, gsp_read_time_ca55_cpu);
    
    /* Verify that CA55 writes and reads to DDR of OTFDE enabled area with DMA transfer. */
    /* Performance is measured during this time. */
    otfde_writeread_verify_ca55_dmac(gsp_ddr_data_single_core, g_plaintext_single_core, SINGLE_CORE_DATA_LEN,
                                    gsp_read_data_ca55_single_core, gsp_write_time_ca55_dmac, gsp_read_time_ca55_dmac);
    
    /* Verify that CR52 and CA55 write and read to DDR of OTFDE enabled area. */
    otfde_writeread_verify_cross_core(gsp_ddr_data_cross_core, g_plaintext_cross_core, CROSS_CORE_DATA_LEN,
                                        gsp_read_data_ca55_cross_core, gsp_compare_result_ca55);
    
    /* Generates an interrupt(INTCPU0) to CR52. */
    err = R_ICU_INTER_CPU_IRQ_Generate(&g_icu_inter_cpu_irq0_ctrl);
    handle_error(err);
    
    /* Close each module and disable interrupts. */
    R_ICU_INTER_CPU_IRQ_Close(&g_icu_inter_cpu_irq1_ctrl);
    
    R_ICU_INTER_CPU_IRQ_Close(&g_icu_inter_cpu_irq0_ctrl);
    
    R_DMAC_Close(&g_transfer1_ctrl);
    
    R_GPT_Close(&g_timer1_ctrl);
    
    __disable_irq();
    
    /* End. */
    while(1);
}

/******************************************************************************
 * @fn       otfde_writeread_verify_ca55_cpu
 * @brief    CA55 accesses DDR with CPU transfer and performance is measured.
 *
 * @param[in]        p_ddr_data            Buffer for writing to DDR.
 * @param[in]        p_plaintext           Plain text.
 * @param[in]        data_len              Data length.
 * @param[in]        p_read_data_from_ddr  Buffer for reading data from DDR.
 * @param[in, out]   p_write_time          Write time via CPU transfer.
 * @param[in, out]   p_read_time           Read time via CPU transfer.
 *
 ******************************************************************************/
static void otfde_writeread_verify_ca55_cpu(uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                            uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    timer_status_t timer;
    int32_t cmp_ret = 0;
    
    /* Write the plain text to DDR with CPU transfer and measure performance. */
    err = R_GPT_Reset(&g_timer1_ctrl);
    handle_error(err);
    
    memset(p_ddr_data, 0, data_len);
    
    err = R_GPT_Start(&g_timer1_ctrl);
    handle_error(err);
    
    memcpy(p_ddr_data, p_plaintext, data_len);
    
    err = R_GPT_Stop(&g_timer1_ctrl);
    handle_error(err);
    
    err = R_GPT_StatusGet(&g_timer1_ctrl, &timer);
    handle_error(err);
    
    *p_write_time = perf_calc_elapsed_time(timer.counter);
    
    err = R_GPT_Reset(&g_timer1_ctrl);
    handle_error(err);
    
    /* Read the data written in DDR from DDR with CPU transfer and measure performance. */
    memset(p_read_data_from_ddr, 0, data_len);
    
    err = R_GPT_Start(&g_timer1_ctrl);
    handle_error(err);
    
    memcpy(p_read_data_from_ddr, p_ddr_data, data_len);
    
    err = R_GPT_Stop(&g_timer1_ctrl);
    handle_error(err);
    
    err = R_GPT_StatusGet(&g_timer1_ctrl, &timer);
    handle_error(err);
    
    *p_read_time = perf_calc_elapsed_time(timer.counter);
    
    /* Check if the read data from DDR matches the plain text. */
    cmp_ret = memcmp(p_read_data_from_ddr, p_plaintext, data_len);
    if (0 != cmp_ret)
    {
        /* When writing and reading with a single core, it is treated as normal only if the comparison results match, */
        /* so processing is stopped as abnormal when the comparison results are inconsistent. */
        while (1);
    }
    
    return;
}

/******************************************************************************
 * @fn       otfde_writeread_verify_ca55_dmac
 * @brief    CA55 accesses DDR with DMA transfer and performance is measured.
 *
 * @param[in]       p_ddr_data           Buffer for writing to DDR.
 * @param[in]       p_plaintext          Plain text.
 * @param[in]       data_len             Data length.
 * @param[in]       p_read_data_from_ddr Buffer for reading data from DDR.
 * @param[in, out]  p_write_time         Write time via DMA transfer.
 * @param[in, out]  p_read_time          Read time via DMA transfer.
 *
 ******************************************************************************/
static void otfde_writeread_verify_ca55_dmac(uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                            uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    timer_status_t timer;
    int32_t cmp_ret = 0;
    
    /* Write the plain text to DDR with DMA transfer and measure performance. */
    err = R_GPT_Reset(&g_timer1_ctrl);
    handle_error(err);
    
    memset(p_ddr_data, 0, data_len);
    
    set_dmac_transfer(p_ddr_data, p_plaintext, data_len);
    
    err = R_GPT_Start(&g_timer1_ctrl);
    handle_error(err);
    
    err = R_DMAC_SoftwareStart(&g_transfer1_ctrl, (transfer_start_mode_t)NULL);
    handle_error(err);
    while (false == gs_dmac_transfer_end_flag);
    
    err = R_GPT_Stop(&g_timer1_ctrl);
    handle_error(err);
    
    gs_dmac_transfer_end_flag = false;
    
    err = R_GPT_StatusGet(&g_timer1_ctrl, &timer);
    handle_error(err);
    
    *p_write_time = perf_calc_elapsed_time(timer.counter);
    
    /* Read the data written in DDR from DDR with DMA transfer and measure performance. */
    err = R_GPT_Reset(&g_timer1_ctrl);
    handle_error(err);
    
    memset(p_read_data_from_ddr, 0, data_len);
    
    set_dmac_transfer(p_read_data_from_ddr, p_ddr_data, data_len);
    
    err = R_GPT_Start(&g_timer1_ctrl);
    handle_error(err);
    
    err = R_DMAC_SoftwareStart(&g_transfer1_ctrl, (transfer_start_mode_t)NULL);
    handle_error(err);
    while (false == gs_dmac_transfer_end_flag);
    
    err = R_GPT_Stop(&g_timer1_ctrl);
    handle_error(err);
    
    gs_dmac_transfer_end_flag = false;
    
    err = R_GPT_StatusGet(&g_timer1_ctrl, &timer);
    handle_error(err);
    
    *p_read_time = perf_calc_elapsed_time(timer.counter);
    
    err = R_GPT_Reset(&g_timer1_ctrl);
    handle_error(err);
    
    /* Check if the read data from DDR matches the plain text. */
    cmp_ret = memcmp(p_read_data_from_ddr, p_plaintext, data_len);
    if (0 != cmp_ret)
    {
        /* When writing and reading with a single core, it is treated as normal only if the comparison results match, */
        /* so processing is stopped as abnormal when the comparison results are inconsistent. */
        while (1);
    }
    
    return;
}

/******************************************************************************
 * @fn       otfde_writeread_verify_cross_core
 * @brief    CR52 and CA55 write and read to DDR.
 *
 * @param[in]       p_ddr_data            Buffer for writing to DDR.
 * @param[in]       p_plaintext           Plain text.
 * @param[in]       data_len              Data length.
 * @param[in]       p_read_data_from_ddr  Buffer for reading data from DDR.
 * @param[in, out]  p_compare_result      Comparison results.
 *
 ******************************************************************************/
static void otfde_writeread_verify_cross_core(uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                            uint8_t * p_read_data_from_ddr, compare_result_t * p_compare_result)
{
    int32_t cmp_ret = 0;
    
    /* Read the data written to DDR by CR52. */
    memset(p_read_data_from_ddr, 0, data_len);
    
    memcpy(p_read_data_from_ddr, p_ddr_data, data_len);
    
    /* Check if the read data from DDR matches the plain text. */
    cmp_ret = memcmp(p_read_data_from_ddr, p_plaintext, data_len);
    if (0 == cmp_ret)
    {
        *p_compare_result = COMPARE_RESULT_MATCH;
    }
    else
    {
        *p_compare_result = COMPARE_RESULT_UNMATCH;
    }
    
    memset(p_ddr_data, 0, data_len);
    
    /* Write the plain text to DDR. */
    memcpy(p_ddr_data, p_plaintext, data_len);
    
    return;
}

/******************************************************************************
 * @fn       set_dmac_transfer
 * @brief    Set DMA transfer settings by using reconfigure API.
 *
 * @param[in]   p_dest     Destination address.
 * @param[in]   p_src      Source address.
 * @param[in]   data_len   Data length to transfer.
 *
 ******************************************************************************/
static void set_dmac_transfer(uint8_t * p_dest, uint8_t * p_src, uint32_t data_len)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    
    extern transfer_info_t g_transfer1_info;
    transfer_info_t transfer_info_1 = g_transfer1_info;
    
    /* Set destination address, source address and length. */
    transfer_info_1.p_dest = (void *)(&p_dest[0]);
    transfer_info_1.p_src  = (void *)(&p_src[0]);
    transfer_info_1.length = data_len;
    
    err = R_DMAC_Reconfigure(&g_transfer1_ctrl, &transfer_info_1);
    handle_error(err);
    
    return;
}

/******************************************************************************
 * @fn       perf_calc_elapsed_time
 * @brief    Calculates elapsed time from counter value.
 *
 * @param[in]   counter         Counter value
 * @param[out]  elapsed_time    Calculated elapsed time
 *
 ******************************************************************************/
static double perf_calc_elapsed_time(uint32_t counter)
{
    double elapsed_time = 0;
    
    /* Check if the clock has wrapped around. */
    if (0 != gs_perf_gpt_wraps_cnt)
    {
        /* Calculate the time for the number of wrap around. */
        elapsed_time = ((double)(GPT_MAX_COUNTS + 1U) / (double)GPT_CLOCK_HZ);
        
        elapsed_time *= (double)gs_perf_gpt_wraps_cnt;
        
        elapsed_time *= 1000;
    }
    
    /* Calculate the total time. */
    elapsed_time += (((double)counter * 1000) / (double)GPT_CLOCK_HZ);
    
    gs_perf_gpt_wraps_cnt = 0;
    
    return elapsed_time;
}

/******************************************************************************
 * @fn handle_error
 * @brief Module error handler.
 *
 * @param[in]  fsp_err        FSP module return code.
 ******************************************************************************/
static void handle_error(fsp_err_t err)
{
    if (FSP_SUCCESS != err)
    {
        /* Error. */
        while (1);
    }
    
    return;
}

/******************************************************************************
 * @fn user_gpt_callback
 * @brief Module error handler.
 *
 * @param[in]  fsp_err        FSP module return code.
 ******************************************************************************/
void user_gpt_callback(timer_callback_args_t * p_args)
{
    if (p_args)
    {
        /* Event for Timer wrap around interrupt. */
        if (TIMER_EVENT_CYCLE_END == p_args->event)
        {
            /* Increment the number of wrap arounds. */
            gs_perf_gpt_wraps_cnt++;
        }
    }
    
    return;
}

/******************************************************************************
 * @fn user_dmac_callback
 * @brief DMAC module callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void user_dmac_callback(transfer_callback_args_t * p_args)
{
    /* Event for transfer end interrupt. */
    if (TRANSFER_EVENT_TRANSFER_END == p_args->event)
    {
        gs_dmac_transfer_end_flag = true;
    }
    
    return;
}

/******************************************************************************
 * @fn cpu1_ic1_callback
 * @brief INTCPU callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void cpu1_ic1_callback(icu_inter_cpu_irq_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    gs_intcpu1_notification_receive_flag = true;
    
    return;
}

