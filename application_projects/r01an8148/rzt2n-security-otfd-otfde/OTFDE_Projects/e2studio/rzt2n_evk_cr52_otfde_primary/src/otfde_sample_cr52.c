/***********************************************************************************************************************
 * File Name    : otfde_sample_cr52.c
 * Description  : Contains data structures and functions used in otfde_sample_cr52.c.
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
#define STR_BUFFER_LEN                   (0x2000U)

#define GPT_CLOCK_HZ                     (400000000U)  /* RZ/T2N GPT counter frequency */
#define GPT_MAX_COUNTS                   (0xFFFFFFFFU) /* GPT count number when wrapping around */

/* DDR of OTFDE enabled area. */
#define DDR_DATA_SINGLE_CORE_ADDR        ((uint32_t)0xE0001000UL)
#define DDR_DATA_CROSS_CORE_ADDR         ((uint32_t)0xE0011000UL)

/* Since CA55 data is used between cores, only the CA55 data storage area is defined. */
#define READ_DATA_CA55_SINGLE_CORE_ADDR  ((uint32_t)0x101E0000UL)
#define READ_DATA_CA55_CROSS_CORE_ADDR   ((uint32_t)0x101F0000UL)

#define WRITE_TIME_CA55_CPU_ADDR         ((uint32_t)0x101F0100UL)
#define WRITE_TIME_CA55_DMAC_ADDR        ((uint32_t)0x101F0200UL)
#define READ_TIME_CA55_CPU_ADDR          ((uint32_t)0x101F0300UL)
#define READ_TIME_CA55_DMAC_ADDR         ((uint32_t)0x101F0400UL)

#define COMPARE_RESULT_CA55_ADDR         ((uint32_t)0x101F0500UL)

#define MSG_SINGLE_CORE_CR52     ("(1) Consistency and Performance Check (CR52)\n\nFlow:\nCR52 --[write]--> DDR_SDRAM --[read]--> CR52\n\n")
#define MSG_SINGLE_CORE_CA55     ("(2) Consistency and Performance Check (CA55)\n\nFlow:\nCA55 --[write]--> DDR_SDRAM --[read]--> CA55\n\n")
#define MSG_CROSS_CORE_CA55_READ ("(3) Inter-Core Encryption and Decryption Check (CR52 -> CA55)\n\nFlow:\nCR52 --[write]--> DDR_SDRAM --[read]--> CA55\n\n")
#define MSG_CROSS_CORE_CR52_READ ("(4) Inter-Core Encryption and Decryption Check (CA55 -> CR52)\n\nFlow:\nCA55 --[write]--> DDR_SDRAM --[read]--> CR52\n\n")

typedef enum e_compare_result
{
    COMPARE_RESULT_MATCH      = 0x00U,
    COMPARE_RESULT_UNMATCH    = 0x01U,
} compare_result_t;

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static char gs_output_str_buffer[STR_BUFFER_LEN] = {0};
static uint32_t gs_perf_gpt_wraps_cnt               = 0;

static uint8_t gs_read_data_cr52_single_core[SINGLE_CORE_DATA_LEN]  = {0};
static uint8_t gs_read_data_cr52_cross_core[CROSS_CORE_DATA_LEN]    = {0};

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

static volatile bool gs_intcpu0_notification_receive_flag = false;
static volatile bool gs_transfer_complete_flag            = false;
static volatile bool gs_dmac_transfer_end_flag            = false;

static void otfde_writeread_verify_cr52_cpu (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                        uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time);
static void otfde_writeread_verify_cr52_dmac (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                        uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time);
static void otfde_writeread_verify_cross_core (uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                                uint8_t * p_read_data_from_ddr, compare_result_t * p_compare_result);
static void output_results_single_core (char * p_msg, uint8_t * p_plaintext, uint32_t data_len, uint8_t * p_read_data_from_ddr,
                            double write_time_cpu, double write_time_dmac, double read_time_cpu, double read_time_dmac);
static void output_results_cross_core (char * p_msg, uint8_t * p_plaintext, uint32_t data_len,
                                        uint8_t * p_read_data_from_ddr, compare_result_t compare_result);

static void set_dmac_transfer (uint8_t * p_dest, uint8_t * p_src, uint32_t data_len);
static double perf_calc_elapsed_time (uint32_t counter);
static void convert_elapsed_time_to_string (double time, char * p_str_buf);
static void strncat_wrapper (char * p_buf, char * p_data);
static void convert_data_to_string (uint8_t * p_data, uint32_t data_len, char * p_str_buf);
static void handle_error (fsp_err_t err);

/******************************************************************************
 * @fn       otfde_sample
 * @brief    OTFDE operation verification process.
 ******************************************************************************/
void otfde_sample(void)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    
    double write_time_cr52_cpu           = 0;
    double read_time_cr52_cpu            = 0;
    double write_time_cr52_dmac          = 0;
    double read_time_cr52_dmac           = 0;
    compare_result_t compare_result_cr52 = COMPARE_RESULT_UNMATCH;
    
    /* Initialize each module and enable interrupts. */
    __enable_irq();
    
    err = R_GPT_Open(&g_timer0_ctrl, &g_timer0_cfg);
    handle_error(err);
    
    err = R_DMAC_Open(&g_transfer0_ctrl, &g_transfer0_cfg);
    handle_error(err);
    
    err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_error(err);
    
    /* Use INTCPU0 to generate a software interrupt from CA55 to CR52. */
    err = R_ICU_INTER_CPU_IRQ_Open(&g_icu_inter_cpu_irq0_ctrl, &g_icu_inter_cpu_irq0_cfg);
    handle_error(err);
    
    /* Use INTCPU1 to generate a software interrupt from CR52 to CA55. */
    err = R_ICU_INTER_CPU_IRQ_Open(&g_icu_inter_cpu_irq1_ctrl, &g_icu_inter_cpu_irq1_cfg);
    handle_error(err);
    
    /* ------------------------------------------------------------- */
    /* In order to execute the primary side first, */
    /* the secondary side will wait for an interrupt to occur after execution. */
    /* Waiting for an interrupt(INTCPU0) from CA55. */
    while (false == gs_intcpu0_notification_receive_flag);
    gs_intcpu0_notification_receive_flag = false;
    /* ------------------------------------------------------------- */
    
    /* Verify that CR52 writes and reads to DDR of OTFDE enabled area with CPU transfer. */
    /* Performance is measured during this time. */
    otfde_writeread_verify_cr52_cpu(gsp_ddr_data_single_core, g_plaintext_single_core, SINGLE_CORE_DATA_LEN,
                                    gs_read_data_cr52_single_core, &write_time_cr52_cpu, &read_time_cr52_cpu);
    
    /* Verify that CR52 writes and reads to DDR of OTFDE enabled area with DMA transfer. */
    /* Performance is measured during this time. */
    otfde_writeread_verify_cr52_dmac(gsp_ddr_data_single_core, g_plaintext_single_core, SINGLE_CORE_DATA_LEN,
                                    gs_read_data_cr52_single_core, &write_time_cr52_dmac, &read_time_cr52_dmac);
    
    /* Output the results of CR52 accessing DDR of OTFDE enabled area. */
    output_results_single_core(MSG_SINGLE_CORE_CR52, g_plaintext_single_core, SINGLE_CORE_DATA_LEN, gs_read_data_cr52_single_core,
                                write_time_cr52_cpu, write_time_cr52_dmac, read_time_cr52_cpu, read_time_cr52_dmac);
    
    /* Verify that CR52 and CA55 write and read to DDR of OTFDE enabled area. */
    /* Within this function, CA55 is notified, and CA55 is processed. */
    otfde_writeread_verify_cross_core(gsp_ddr_data_cross_core, g_plaintext_cross_core, CROSS_CORE_DATA_LEN,
                                        gs_read_data_cr52_cross_core, &compare_result_cr52);
    
    /* Outputs the result of CA55 accessing DDR of OTFDE enabled area. */
    output_results_single_core(MSG_SINGLE_CORE_CA55, g_plaintext_cross_core, CROSS_CORE_DATA_LEN, gsp_read_data_ca55_single_core,
                                *gsp_write_time_ca55_cpu, *gsp_write_time_ca55_dmac, *gsp_read_time_ca55_cpu, *gsp_read_time_ca55_dmac);
    
    /* Outputs the results of CR52 and CA55 accessing DDR of OTFDE enabled area. */
    output_results_cross_core(MSG_CROSS_CORE_CA55_READ, g_plaintext_cross_core, CROSS_CORE_DATA_LEN,
                            gsp_read_data_ca55_cross_core, *gsp_compare_result_ca55);
    
    output_results_cross_core(MSG_CROSS_CORE_CR52_READ, g_plaintext_cross_core, CROSS_CORE_DATA_LEN,
                            gs_read_data_cr52_cross_core, compare_result_cr52);
    
    /* Close each module and disable interrupts. */
    R_ICU_INTER_CPU_IRQ_Close(&g_icu_inter_cpu_irq1_ctrl);
    
    R_ICU_INTER_CPU_IRQ_Close(&g_icu_inter_cpu_irq0_ctrl);
    
    R_SCI_UART_Close(&g_uart0_ctrl);
    
    R_DMAC_Close(&g_transfer0_ctrl);
    
    R_GPT_Close(&g_timer0_ctrl);
    
    __disable_irq();
    
    /* End. */
    while(1);
}

/******************************************************************************
 * @fn       otfde_writeread_verify_cr52_cpu
 * @brief    CR52 accesses DDR with CPU transfer and performance is measured.
 *
 * @param[in]        p_ddr_data            Buffer for writing to DDR.
 * @param[in]        p_plaintext           Plain text.
 * @param[in]        data_len              Data length.
 * @param[in]        p_read_data_from_ddr  Buffer for reading data from DDR.
 * @param[in, out]   p_write_time          Write time via CPU transfer.
 * @param[in, out]   p_read_time           Read time via CPU transfer.
 *
 ******************************************************************************/
static void otfde_writeread_verify_cr52_cpu(uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                            uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    timer_status_t timer;
    int32_t cmp_ret = 0;
    
    /* Write the plain text to DDR with CPU transfer and measure performance. */
    err = R_GPT_Reset(&g_timer0_ctrl);
    handle_error(err);
    
    memset(p_ddr_data, 0, data_len);
    
    err = R_GPT_Start(&g_timer0_ctrl);
    handle_error(err);
    
    memcpy(p_ddr_data, p_plaintext, data_len);
    
    err = R_GPT_Stop(&g_timer0_ctrl);
    handle_error(err);
    
    err = R_GPT_StatusGet(&g_timer0_ctrl, &timer);
    handle_error(err);
    
    *p_write_time = perf_calc_elapsed_time(timer.counter);
    
    err = R_GPT_Reset(&g_timer0_ctrl);
    handle_error(err);
    
    /* Read the data written in DDR from DDR with CPU transfer and measure performance. */
    memset(p_read_data_from_ddr, 0, data_len);
    
    err = R_GPT_Start(&g_timer0_ctrl);
    handle_error(err);
    
    memcpy(p_read_data_from_ddr, p_ddr_data, data_len);
    
    err = R_GPT_Stop(&g_timer0_ctrl);
    handle_error(err);
    
    err = R_GPT_StatusGet(&g_timer0_ctrl, &timer);
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
 * @fn       otfde_writeread_verify_cr52_dmac
 * @brief    CR52 accesses DDR with DMA transfer and performance is measured.
 *
 * @param[in]       p_ddr_data           Buffer for writing to DDR.
 * @param[in]       p_plaintext          Plain text.
 * @param[in]       data_len             Data length.
 * @param[in]       p_read_data_from_ddr Buffer for reading data from DDR.
 * @param[in, out]  p_write_time         Write time via DMA transfer.
 * @param[in, out]  p_read_time          Read time via DMA transfer.
 *
 ******************************************************************************/
static void otfde_writeread_verify_cr52_dmac(uint8_t * p_ddr_data, uint8_t * p_plaintext, uint32_t data_len,
                                            uint8_t * p_read_data_from_ddr, double * p_write_time, double * p_read_time)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    timer_status_t timer;
    int32_t cmp_ret = 0;
    
    /* Write the plain text to DDR with DMA transfer and measure performance. */
    err = R_GPT_Reset(&g_timer0_ctrl);
    handle_error(err);
    
    memset(p_ddr_data, 0, data_len);
    
    set_dmac_transfer(p_ddr_data, p_plaintext, data_len);
    
    err = R_GPT_Start(&g_timer0_ctrl);
    handle_error(err);
    
    err = R_DMAC_SoftwareStart(&g_transfer0_ctrl, (transfer_start_mode_t)NULL);
    handle_error(err);
    while (false == gs_dmac_transfer_end_flag);
    
    err = R_GPT_Stop(&g_timer0_ctrl);
    handle_error(err);
    
    gs_dmac_transfer_end_flag = false;
    
    err = R_GPT_StatusGet(&g_timer0_ctrl, &timer);
    handle_error(err);
    
    *p_write_time = perf_calc_elapsed_time(timer.counter);
    
    /* Read the data written in DDR from DDR with DMA transfer and measure performance. */
    err = R_GPT_Reset(&g_timer0_ctrl);
    handle_error(err);
    
    memset(p_read_data_from_ddr, 0, data_len);
    
    set_dmac_transfer(p_read_data_from_ddr, p_ddr_data, data_len);
    
    err = R_GPT_Start(&g_timer0_ctrl);
    handle_error(err);
    
    err = R_DMAC_SoftwareStart(&g_transfer0_ctrl, (transfer_start_mode_t)NULL);
    handle_error(err);
    while (false == gs_dmac_transfer_end_flag);
    
    err = R_GPT_Stop(&g_timer0_ctrl);
    handle_error(err);
    
    gs_dmac_transfer_end_flag = false;
    
    err = R_GPT_StatusGet(&g_timer0_ctrl, &timer);
    handle_error(err);
    
    *p_read_time = perf_calc_elapsed_time(timer.counter);
    
    err = R_GPT_Reset(&g_timer0_ctrl);
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
    fsp_err_t err = FSP_ERR_ASSERTION;
    int32_t cmp_ret = 0;
    
    /* Write the plain text to DDR. */
    memset(p_ddr_data, 0, data_len);
    
    memcpy(p_ddr_data, p_plaintext, data_len);
    
    /* Generates an interrupt(INTCPU1) to CA55. */
    err = R_ICU_INTER_CPU_IRQ_Generate(&g_icu_inter_cpu_irq1_ctrl);
    handle_error(err);
    
    /* Waiting for an interrupt(INTCPU0) from CA55. */
    while (false == gs_intcpu0_notification_receive_flag);
    gs_intcpu0_notification_receive_flag = false;
    
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
    
    return;
}

/******************************************************************************
 * @fn       output_results_single_core
 * @brief    Output the results of the check using single core.
 *
 * @param[in]   p_msg                    Output message.
 * @param[in]   p_plaintext              Plain text.
 * @param[in]   data_len                 Data length.
 * @param[in]   p_read_data_from_ddr     Data read from DDR.
 * @param[in]   write_time_cpu           Write time via CPU transfer.
 * @param[in]   write_time_dmac          Write time via DMA transfer.
 * @param[in]   read_time_cpu            Read time via CPU transfer.
 * @param[in]   read_time_dmac           Read time via DMA transfer.
 *
 ******************************************************************************/
static void output_results_single_core(char * p_msg, uint8_t * p_plaintext, uint32_t data_len, uint8_t * p_read_data_from_ddr,
                double write_time_cpu, double write_time_dmac, double read_time_cpu, double read_time_dmac)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    
    memset(gs_output_str_buffer, 0, STR_BUFFER_LEN);
    
    /* Store the messages to the send buffer. */
    strncat_wrapper(gs_output_str_buffer, p_msg);
    
    /* Output to the log only if the comparison results match the plain text values. */
    /* Therefore, the results of comparisons are always consistent. */
    strncat_wrapper(gs_output_str_buffer, "Comparison result: Match\n");
    
    /* Store plain text in the send buffer. */
    strncat_wrapper(gs_output_str_buffer, "Text to be written\n");
    convert_data_to_string(p_plaintext, data_len, gs_output_str_buffer);
    
    /* DDR readout results during DMA transfer (DDR readout area is used by CPU and DMA). */
    strncat_wrapper(gs_output_str_buffer, "Text read from DDR\n");
    convert_data_to_string(p_read_data_from_ddr, data_len, gs_output_str_buffer);
    strncat_wrapper(gs_output_str_buffer, "\n");
    
    /* Store performance measurement results in the send buffer. */
    strncat_wrapper(gs_output_str_buffer, "Performance results:\n");
    
    strncat_wrapper(gs_output_str_buffer, "Write time\nCPU transfer : ");
    convert_elapsed_time_to_string(write_time_cpu, gs_output_str_buffer);
    
    strncat_wrapper(gs_output_str_buffer, "DMA transfer : ");
    convert_elapsed_time_to_string(write_time_dmac, gs_output_str_buffer);
    
    strncat_wrapper(gs_output_str_buffer, "Read time\nCPU transfer : ");
    convert_elapsed_time_to_string(read_time_cpu, gs_output_str_buffer);
    
    strncat_wrapper(gs_output_str_buffer, "DMA transfer : ");
    convert_elapsed_time_to_string(read_time_dmac, gs_output_str_buffer);
    
    strncat_wrapper(gs_output_str_buffer, "\n");
    
    /* Output the results. */
    err = R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *)gs_output_str_buffer, strlen(gs_output_str_buffer));
    handle_error(err);
    while (false == gs_transfer_complete_flag);
    gs_transfer_complete_flag = false;
    
    return;
}

/******************************************************************************
 * @fn       output_results_cross_core
 * @brief    Output the results of the check using cross core.
 *
 * @param[in]   p_msg                    Output message.
 * @param[in]   p_plaintext              Plain text.
 * @param[in]   data_len                 Data length.
 * @param[in]   p_read_data_from_ddr     Data read from DDR.
 * @param[in]   compare_result           Comparison results.
 *
 ******************************************************************************/
static void output_results_cross_core(char * p_msg, uint8_t * p_plaintext, uint32_t data_len,
                                    uint8_t * p_read_data_from_ddr, compare_result_t compare_result)
{
    fsp_err_t err = FSP_ERR_ASSERTION;
    
    memset(gs_output_str_buffer, 0, STR_BUFFER_LEN);
    
    /* Store the messages to the send buffer. */
    strncat_wrapper(gs_output_str_buffer, p_msg);
    
    /* Store comparison results. */
    if (COMPARE_RESULT_MATCH == compare_result)
    {
        strncat_wrapper(gs_output_str_buffer, "Comparison result: Match\n");
    }
    else
    {
        strncat_wrapper(gs_output_str_buffer, "Comparison result: Unmatch\n");
    }
    
    /* Store plaintext in the send buffer. */
    strncat_wrapper(gs_output_str_buffer, "Text to be written\n");
    convert_data_to_string(p_plaintext, data_len, gs_output_str_buffer);
    
    /* Store the read data written to DDR by each other core. */
    strncat_wrapper(gs_output_str_buffer, "Text read from DDR\n");
    convert_data_to_string(p_read_data_from_ddr, data_len, gs_output_str_buffer);
    strncat_wrapper(gs_output_str_buffer, "\n");
    
    /* Output the results. */
    err = R_SCI_UART_Write(&g_uart0_ctrl, (uint8_t *)gs_output_str_buffer, strlen(gs_output_str_buffer));
    handle_error(err);
    while (false == gs_transfer_complete_flag);
    gs_transfer_complete_flag = false;
    
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
    
    extern transfer_info_t g_transfer0_info;
    transfer_info_t transfer_info_0 = g_transfer0_info;
    
    /* Set destination address, source address and length. */
    transfer_info_0.p_dest = (void *)(&p_dest[0]);
    transfer_info_0.p_src  = (void *)(&p_src[0]);
    transfer_info_0.length = data_len;
    
    err = R_DMAC_Reconfigure(&g_transfer0_ctrl, &transfer_info_0);
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
 * @fn       convert_elapsed_time_to_string
 * @brief    Convert elapsed time to a string.
 *
 * @param[in]       time         Elapsed time to be converted
 * @param[in, out]  p_str_buf    Buffer to store the converted string
 *
 ******************************************************************************/
static void convert_elapsed_time_to_string(double time, char * p_str_buf)
{
    char str_data[1000] = {0};
    
    int32_t written = snprintf(str_data, 1000, "%.7f ms\n", time);
    if (written <= 0)
    {
        /* Failed. */
        while (1);
    }
    
    strncat_wrapper(p_str_buf, str_data);
    
    return;
}

/******************************************************************************
 * @fn       convert_data_to_string
 * @brief    Convert the first and last 4 bytes of the data into a string.
 *
 * @param[in]       p_data         Data to convert
 * @param[in]       data_len       Data length to convert
 * @param[in, out]  p_str_buf      Buffer to store the converted string
 *
 ******************************************************************************/
static void convert_data_to_string(uint8_t * p_data, uint32_t data_len, char * p_str_buf)
{
    /* Convert the first 4 bytes of the data. */
    for (uint32_t i = 0; i < 4; i++)
    {
        char str_data[16] = {0};
        
        int32_t written = snprintf(str_data, 16, "0x%02X, ", p_data[i]);
        if (written <= 0)
        {
            /* Failed. */
            while (1);
        }
        
        strncat_wrapper(p_str_buf, str_data);
        
    }
    
    /* Data other than the first and last 4 bytes is omitted. */
    strncat_wrapper(p_str_buf, "..., ");
    
    /* Convert the last 4 bytes of data. */
    uint8_t * p_tail_pos = (p_data+data_len)-4;
    for (uint32_t i = 0; i < 4; i++)
    {
        char str_data[16] = {0};
        
        int32_t written = snprintf(str_data, 16, "0x%02X, ", p_tail_pos[i]);
        if (written <= 0)
        {
            /* Failed. */
            while (1);
        }
        
        strncat_wrapper(p_str_buf, str_data);
        
    }
    
    strncat_wrapper(p_str_buf, "\n");
    
    return;
}

/******************************************************************************
 * @fn       strncat_wrapper
 * @brief    Concatenate strings.
 * @param[in, out]  p_buf          Concatenate String
 * @param[in]       p_data         Strings to be concatenated
 *
 ******************************************************************************/
static void strncat_wrapper(char * p_buf, char * p_data)
{
    size_t used = strlen(p_buf);
    
    /* Calculate the available size including null-terminating string. */
    size_t available_size = (STR_BUFFER_LEN-1)-used ;
    
    /* Check if the string to be concatenated fits in the buffer. */
    if (available_size < strlen(p_data))
    {
        /* Failed. */
        while (1);
    }
    
    /* Concatenate strings. */
    strncat(p_buf, p_data, available_size);
    
    return;
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
 * @fn cpu0_ic0_callback
 * @brief INTCPU callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void cpu0_ic0_callback(icu_inter_cpu_irq_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    gs_intcpu0_notification_receive_flag = true;
    
    return;
}

/******************************************************************************
 * @fn user_uart_callback
 * @brief SCI UART module callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void user_uart_callback(uart_callback_args_t * p_args)
{
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
            gs_transfer_complete_flag = true;
            break;
            
        default:
            break;
    }
    
    return;
}

