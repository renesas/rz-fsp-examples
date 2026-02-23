/***********************************************************************************************************************
 * File Name    : xspi_ep.c
 * Description  : Contains data structures and functions used in xspi_ep.c.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2023 - 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#include "xspi_ep.h"
#include "common_utils.h"

/***********************************************************************************************************************
 * @addtogroup r_xspi_ep
 * @{
 **********************************************************************************************************************/

uint8_t g_freq_str[6] = {RESET_VALUE};

/***********************************************************************************************************************
 * Function Name: hram_example
 * Arguments    : none
 * Return Value : fsp_err_t err
 **********************************************************************************************************************/
fsp_err_t hram_example(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* MDVn pin should be low(Set XSPI operating voltage at 1.8V) */
    uint32_t mdv = (R_SYSC_NS->MD_MON >> 16) & 0x1f;
    if ((0 << MDVn) != (mdv & (1 << MDVn)))
    {
        APP_PRINT("  ERROR: MDV%d pin should be low. \n", MDVn);
        return FSP_ERR_ABORTED;
    }

    /* XSPI initial settings */
    R_XSPI_HYPER_Open(&g_hyperbus0_ctrl, &g_hyperbus0_cfg);
    if (FSP_SUCCESS != err)
    {   /* XSPI instance open failed */
        APP_PRINT("\r\n ** R_XSPI_HYPER OPEN FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }

    /* Checking XSPI initial settings (checking registers) */
    APP_PRINT("\r\n  Check xSPI register settings\n");
    xspi_hram_init_check(&g_hyperbus0_cfg);

    /* HyperRAM memory test */
    err = hram_memory_test(&g_hyperbus0_cfg);
    if (FSP_SUCCESS != err)
    {   /* HyperRAM memory test failed */
        APP_PRINT("\r\n ** HRAM MEMORY TEST FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }

    /* Read HyperRAM register(The data read is big endian.) */
    /* Read setting of identification register 0 */
    APP_PRINT("\r\n  Read HyperRAM register\r\n");
    volatile hram_transfer_t trans[2];
    trans[0].cmd     = HRAM_DEVICE_READ_CMD;
    trans[0].addr    = HRAM_DEVICE_ID0_ADDR;
    trans[0].latency = 6;
    trans[0].write   = false;
    err              = hram_trans(&g_hyperbus0_cfg, trans[0]);
    if (FSP_SUCCESS != err)
    {   /* HyperRAM transfer failed */
        APP_PRINT("\r\n ** HRAM TRANSFER FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }

    /* Since the data is big endian, convert the endian and output. */
    APP_PRINT("    Device ID register0     : 0x%04X\r\n", swap16((uint16_t)R_XSPIn->BUF[0].CDD0));

    /* Read setting of identification register 1 */
    trans[0].addr = HRAM_DEVICE_ID1_ADDR;
    err           = hram_trans(&g_hyperbus0_cfg, trans[0]);
    if (FSP_SUCCESS != err)
    {   /* HyperRAM transfer failed */
        APP_PRINT("\r\n ** HRAM TRANSFER FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }
    APP_PRINT("    Device ID register1     : 0x%04X\r\n", swap16((uint16_t)R_XSPIn->BUF[0].CDD0));

    /* Read setting of configuration register 0 */
    trans[0].addr = HRAM_DEVICE_CFG0_ADDR;
    err           = hram_trans(&g_hyperbus0_cfg, trans[0]);
    if (FSP_SUCCESS != err)
    {   /* HyperRAM transfer failed */
        APP_PRINT("\r\n ** HRAM TRANSFER FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }
    APP_PRINT("    Configuration Register0 : 0x%04X\r\n", swap16((uint16_t)R_XSPIn->BUF[0].CDD0));

    /* Read setting of configuration register 1 */
    trans[0].addr = HRAM_DEVICE_CFG1_ADDR;
    err           = hram_trans(&g_hyperbus0_cfg, trans[0]);
    if (FSP_SUCCESS != err)
    {   /* HyperRAM transfer failed */
        APP_PRINT("\r\n ** HRAM TRANSFER FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }
    APP_PRINT("    Configuration Register1 : 0x%04X\r\n", swap16((uint16_t)R_XSPIn->BUF[0].CDD0));
    return err;
}

/***********************************************************************************************************************
 * Function Name: xspi_hram_init_check
 * Description  : Check xspi register settings
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
void xspi_hram_init_check(hyperbus_cfg_t const * const p_cfg)
{
    /* Conversion from generic p_extend to HyperRAM specific extended config type */
    xspi_hyper_extended_cfg_t * p_cfg_extend = (xspi_hyper_extended_cfg_t *) p_cfg->p_extend;

    /* Cast register value to unsigned integer */
    APP_PRINT("    CSSCTL    : 0x%08X\n", (unsigned int)R_XSPIn->CSSCTL);
    APP_PRINT("    WRAPCFG   : 0x%08X\n", (unsigned int)R_XSPIn->WRAPCFG);
    APP_PRINT("    LIOCFGCSn : 0x%08X\n", (unsigned int)R_XSPIn->LIOCFGCS[p_cfg_extend->chip_select]);
    APP_PRINT("    COMCFG    : 0x%08X\n", (unsigned int)R_XSPIn->COMCFG);
    APP_PRINT("    BMCFG     : 0x%08X\n", (unsigned int)R_XSPIn->BMCFG);
    APP_PRINT("    CMCFG0    : 0x%08X\n", (unsigned int)R_XSPIn->CSa[p_cfg_extend->chip_select].CMCFG0);
    APP_PRINT("    CMCFG1    : 0x%08X\n", (unsigned int)R_XSPIn->CSa[p_cfg_extend->chip_select].CMCFG1);
    APP_PRINT("    CMCFG2    : 0x%08X\n", (unsigned int)R_XSPIn->CSa[p_cfg_extend->chip_select].CMCFG2);
    APP_PRINT("    CMCTL     : 0x%08X\n", (unsigned int)R_XSPIn->CMCTL);
    APP_PRINT("    BMCTL0    : 0x%08X\n", (unsigned int)R_XSPIn->BMCTL0);
    APP_PRINT("    BMCTL1    : 0x%08X\n", (unsigned int)R_XSPIn->BMCTL1); /* BMCTL1 returns 0x00000000 for write only */
}

/***********************************************************************************************************************
 * Function Name: hram_trans
 * Description  : Transfer HyperRAM
 * Arguments    : hram_transfer_t trans
 * Return Value : fsp_err_t err
 **********************************************************************************************************************/
fsp_err_t hram_trans(hyperbus_cfg_t const * const p_cfg, hram_transfer_t trans)
{
    /* Conversion from generic p_extend to HyperRAM specific extended config type */
    xspi_hyper_extended_cfg_t * p_cfg_extend = (xspi_hyper_extended_cfg_t *) p_cfg->p_extend;

    fsp_err_t err = FSP_SUCCESS;

    /* xSPI Command Manual Type Buf */
    R_XSPIn->BUF[0].CDT_b.CMDSIZE = 2;  /* Command Size: 2 bytes */
    R_XSPIn->BUF[0].CDT_b.ADDSIZE = 4;  /* Address Size: 4 bytes */
    R_XSPIn->BUF[0].CDT_b.DATASIZE = 2; /* Write/Read Data Size: 2 bytes */
    R_XSPIn->BUF[0].CDT_b.LATE = (uint8_t)(trans.latency & 0x1Fu);  /* LATE is a 5-bit field; mask to 5 bits to keep range valid */
    R_XSPIn->BUF[0].CDT_b.TRTYPE = trans.write; /* Transaction Type */
    R_XSPIn->BUF[0].CDT_b.CMD = trans.cmd;      /* Command (1-2 bytes) */
/* xSPI Command Manual Address Buf */
    R_XSPIn->BUF[0].CDA  = trans.addr;

    /* For write transactions, write data to buffer registers */
    if (true == trans.write)
    {
        R_XSPIn->BUF[0].CDD0 = trans.data;
    }

    /* xSPI Command Manual Control Register 0 */
    R_XSPIn->CDCTL0_b.CSSEL = p_cfg_extend->chip_select; /* Chip select */
    R_XSPIn->CDCTL0_b.TRNUM = 0;  /* Transaction number: Issue 1 command (using command buffer 0) */
    R_XSPIn->CDCTL0_b.PERMD = 0;  /* Periodic mode: Direct manual-command mode */
    R_XSPIn->CDCTL0_b.PERITV = 0; /* Periodic transaction interval: 2 cycles */
    FSP_HARDWARE_REGISTER_WAIT(R_XSPIn->CDCTL0_b.TRREQ, 0); /* Wait for transaction completion */
    R_XSPIn->CDCTL0_b.TRREQ = 1;  /* Transaction request: Request transaction */
/* xSPI Interrupt Status Register */
    FSP_HARDWARE_REGISTER_WAIT(R_XSPIn->INTS_b.CMDCMP, 1);  /* Wait for requested manual command completion */
/* xSPI Interrupt Clear Register */
    R_XSPIn->INTC_b.CMDCMPC = 1;  /* Command Completed interrupt clear: Clear interrupt status */
/* DS Timeout and AHB Bus Error Detection */
    err = check_ints(p_cfg);
    if (FSP_SUCCESS != err)
    {   /* DS Timeout and AHB Bus Error Detection failed */
        APP_PRINT("\r\n ** DETECTION FAILED ** \r\n");
        APP_ERR_TRAP(err);
    }

    /* For read transactions, read data from buffer registers. */
    if (false == trans.write)
    {
        trans.data = R_XSPIn->BUF[0].CDD0;
    }

    return err;
}

/***********************************************************************************************************************
 * Function Name: hram_memory_test
 * Description  : HyperRAM memory test
 * Arguments    : none
 * Return Value : fsp_err_t err
 **********************************************************************************************************************/
fsp_err_t hram_memory_test(hyperbus_cfg_t const * const p_cfg)
{
    /* Conversion from generic p_extend to HyperRAM specific extended config type */
    xspi_hyper_extended_cfg_t * p_cfg_extend = (xspi_hyper_extended_cfg_t *) p_cfg->p_extend;

    fsp_err_t err = FSP_SUCCESS;

    /* Check XSPIx clock frequency */
    double freq;
    switch (XSPIn_CLOCK)
    {
        case 2:
            freq = 133;
            break;
        case 3:
            freq = 100;
            break;
        case 4:
            freq = 50;
            break;
        case 5:
            freq = 25;
            break;
        case 6:
            freq = 12.5;
            break;
        default:
            freq = -1;
            break;
    }

    /* Conversion to char pointer for string formatting */
    snprintf((char *)g_freq_str, sizeof g_freq_str, "%.2f", freq);
    APP_PRINT("\r\n  XSPI%d Clock = %s MHz\r\n", p_cfg_extend->unit, g_freq_str);
    APP_PRINT("\r\n  HyperRAM memory test Starts.");

    /* HyperRAM memory mapped test */
    volatile uint32_t temp;
    uint32_t *        p_word;
    uint32_t          test_pat[TPAT_NUM] =
    {
        0xffffffff, 0xaaaaaaaa, 0x00000000, 0x55555555,
    };

    for (uint8_t j = 0; j < LOOP_ITER; j++)
    {
        /* Write */
        p_word = (uint32_t *) HRAM_DEVICE_START_ADDRESS;
        for (int i = 0; i < HRAM_DEVICE_SIZE/4; i++)
        {
            *p_word++ = test_pat[j % TPAT_NUM];
            err = check_ints(p_cfg);
            while(FSP_SUCCESS != err)
            {
                return err;
            }
        }

        /* Read-Write-Read */
        p_word--;
        for (int i = HRAM_DEVICE_SIZE/4; i > 0; i--)
        {
            temp = *p_word;

            if ( temp != test_pat[j % TPAT_NUM])
            {
                APP_PRINT("  Fail-Addr:%lx Data:%lx.\n", (uint32_t)p_word, temp);
                return FSP_ERR_ABORTED;
            }

            *p_word = test_pat[(j+1) % TPAT_NUM];
            temp = *p_word--;
            __asm("dsb");
            __asm("isb");

            if ( temp != test_pat[(j+1) % TPAT_NUM])
            {
                APP_PRINT("  Fail-Addr:%lx Data:%lx.\n", (uint32_t)p_word, temp);
                return FSP_ERR_ABORTED;
            }
        }
        APP_PRINT(".");
    }
    APP_PRINT("\r\n  HyperRAM memory test Ends!\n");

    return err;
}

/***********************************************************************************************************************
 * Function Name: check_ints
 * Description  : DS Timeout and AHB Bus Error Detection
 * Arguments    : none
 * Return Value : none
 **********************************************************************************************************************/
fsp_err_t check_ints(hyperbus_cfg_t const * const p_cfg)
{
    /* Conversion from generic p_extend to HyperRAM specific extended config type */
    xspi_hyper_extended_cfg_t * p_cfg_extend = (xspi_hyper_extended_cfg_t *) p_cfg->p_extend;

    /* AHB bus error detection */
    if (R_XSPIn->INTS_b.BUSERR)
    {
        APP_PRINT("  ERROR: Bus Error!\n");
        R_XSPIn->INTC_b.BUSERRC = 1;
        return FSP_ERR_ABORTED;
    }

    /* DS timeout detection */
    if (XSPIn_DSTOCS)
    {
        APP_PRINT("  ERROR: DS timeout! (CS%d)\n", p_cfg_extend->chip_select);
        XSPIn_DSTOCS_C = 1;
        return FSP_ERR_ABORTED;
    }

    return FSP_SUCCESS;
}

/***********************************************************************************************************************
 * Function Name: swap16
 * Description  : Convert endian
 * Arguments    : uint16_t value
 * Return Value : return ret
 **********************************************************************************************************************/
uint16_t swap16(uint16_t value)
{
    uint16_t ret;
    ret  = (value << 8);
    ret |= (value >> 8);
    return ret;
}

/***********************************************************************************************************************
 * @} (end addtogroup r_xspi_ep)
 **********************************************************************************************************************/
