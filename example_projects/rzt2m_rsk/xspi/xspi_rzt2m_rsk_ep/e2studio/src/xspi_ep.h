/***********************************************************************************************************************
 * File Name    : xspi_ep.h
 * Description  : Contains declarations of data structures and functions used in xspi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2022 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef XSPI_EP_H_
#define XSPI_EP_H_

#include "hal_data.h"

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef struct {
    uint16_t cmd;
    uint32_t addr;
    uint32_t data;
    uint32_t latency;
    bool write;
} hram_transfer_t;

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define R_XSPIn                      (R_XSPI1)
#define XSPIn_DSTOCS                 (R_XSPIn->INTS_b.DSTOCS0)
#define XSPIn_DSTOCS_C               (R_XSPIn->INTC_b.DSTOCS0C)
#define XSPIn_CLOCK                  (R_SYSC_NS->SCKCR_b.FSELXSPI1)
#define MDVn                         (4)
#define HRAM_DEVICE_START_ADDRESS    (0x48000000UL)
#define TPAT_NUM                     (4)
#define LOOP_ITER                    (TPAT_NUM * 1)
#define HRAM_DEVICE_SIZE             (0x800000)
#define HRAM_DEVICE_READ_CMD         (0xc000)
#define HRAM_DEVICE_ID0_ADDR         (0x00000000)
#define HRAM_DEVICE_ID1_ADDR         (0x00000001)
#define HRAM_DEVICE_CFG0_ADDR        (0x01000000)
#define HRAM_DEVICE_CFG1_ADDR        (0x01000001)

/* Macros for example project info to be displayed */
#define EP_INFO           "\r\nThis Example Project demonstrates the xSPI HyperBus driver with an external HyperRAM on CS0 of xSPI1."\
                          "\r\nAt start-up, open the xSPI HyperBus instance, and dumps key register settings for sanity check."\
                          "\r\nA memory-mapped test writes/reads pattern data across the HyperRAM range while checking for AHB bus errors and DS timeouts."\
                          "\r\nThe demo then issues manual-command reads of Device ID Registers and Configuration registers, converting from big-endian before printing."\
                          "\r\nThe xSPI clock and overall pass/fail status are displayed on the RTT Viewer.\r\n"\

/***********************************************************************************************************************
 * Functions Prototypes
 **********************************************************************************************************************/
fsp_err_t hram_example(void);
fsp_err_t hram_trans(hyperbus_cfg_t const * const p_cfg, hram_transfer_t trans);
fsp_err_t hram_memory_test(hyperbus_cfg_t const * const p_cfg);
fsp_err_t check_ints(hyperbus_cfg_t const * const p_cfg);
void xspi_hram_init_check(hyperbus_cfg_t const * const p_cfg);
uint16_t swap16(uint16_t value);

#endif /* XSPI_EP_H_ */
