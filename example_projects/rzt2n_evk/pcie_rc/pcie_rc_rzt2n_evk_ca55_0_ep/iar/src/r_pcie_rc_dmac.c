/***********************************************************************************************************************
 * File Name    : r_pcie_rc_dmac.c
 * Description  : PCIe RC DMA controller configuration and control functions.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

/******************************************************************************
 Includes
 ******************************************************************************/
#include "hal_data.h"
#include "r_pcie_rc_dmac.h"

/******************************************************************************
 Exported global functions
 ******************************************************************************/
/*******************************************************************************************************************//**
 * @brief       Configures the PCIe RC DMA channel registers (source, destination, size, transfer direction) for a
 *              subsequent DMA transfer.
 * @param[in]   p_ctrl           Pointer to the PCIe RC control block
 * @param[in]   dmach            DMA channel number
 * @param[in]   transfar_mode    Transfer direction (AXItoPCI or PCItoAXI)
 * @param[in]   src_addr         Source address for the DMA transfer
 * @param[in]   dest_addr        Destination address for the DMA transfer
 * @param[in]   write_buf        Transfer size written to the DMA size register
 * @retval      FSP_SUCCESS             Upon successful configuration
 * @retval      FSP_ERR_UNSUPPORTED     Invalid DMA channel or unsupported transfer mode
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Function Name: R_PCIE_RC_DmacSet
 * Description  :
 * Arguments    :
 * Return Value :
 **********************************************************************************************************************/
fsp_err_t R_PCIE_RC_DmacSet (
    pci_ctrl_t * const p_ctrl,
    uint8_t            dmach,
    uint32_t           transfar_mode,
    uint64_t           src_addr,
    uint64_t           dest_addr,
    uint32_t           write_buf)
{
    /* Get PCIe RC driver instance */
    pcie_rc_instance_ctrl_t * p_instance_ctrl = (pcie_rc_instance_ctrl_t *) p_ctrl;

    /* PCIe register base address */
    uint32_t pcie_base_address = 0;

    /* DMA channel register offset (0x80 bytes per channel) */
    uint32_t dmac_ch_offset = 0x80U * (uint32_t) dmach;

    uint32_t u_data;
    uint32_t l_data;

    pci_configuration_register_transfer_t pcie_rc0_cfg_reg_read =
    {
        .header_word_b.ext_register_number  = 0U,
        .header_word_b.function_number      = 0U,
        .header_word_b.bus_number           = 0U,
    };

    pci_configuration_register_transfer_t pcie_rc0_cfg_reg_write =
    {
        .header_word_b.ext_register_number  = 0U,
        .header_word_b.function_number      = 0U,
        .header_word_b.bus_number           = 0U,
    };

    pcie_base_address = R_PCIE0_BASE;

    if ((dmach) > (7))
    {
        return FSP_ERR_UNSUPPORTED;
    }

    /* memory bus enable (EP) */
    /* Read the PCI_EP_COM_STA_F0 configuration register. */
    pcie_rc0_cfg_reg_read.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    R_PCIE_RC_ConfigurationRegisterRead(&g_pci_rc0_ctrl, PCI_CONFIGURATION_READ_TYPE_0, &pcie_rc0_cfg_reg_read);

    /* Set the memory space and bus master enable bits and write the register back. */
    pcie_rc0_cfg_reg_write.header_word_b.register_number = 1U;  /* PCI_EP_COM_STA_F0 */
    pcie_rc0_cfg_reg_write.data = pcie_rc0_cfg_reg_read.data | 0x00000006;
    R_PCIE_RC_ConfigurationRegisterWrite(&g_pci_rc0_ctrl, PCI_CONFIGURATION_WRITE_TYPE_0, &pcie_rc0_cfg_reg_write);

    /* memory bus enable (RC) */
    /* Set the memory space and bus master enable bits on the RC side. */
    R_PCIE0->PCI_EP_COM_STA_F0 |= 0x00000006;

    /** Set the registers */
    /* Use the default 128-byte DMA burst size. */
    p_instance_ctrl->p_reg->PCI_RC_DMACTRL = 0x00000000; /* 128 bytes(default) */
    /* Clear all pending DMA interrupt status bits. */
    p_instance_ctrl->p_reg->PCI_RC_DMAINTS = 0xFFFFFFFF;
    /* Enable the error, queue-empty, stop, and end interrupts for this DMA channel. */
    /* CHn_ERR_EN, CHn_QUE_EMP_EN, CHn_STOP_EN, CHn_END_EN (n = dmach) */
    p_instance_ctrl->p_reg->PCI_RC_DMAINTE = (0xF << (4 * dmach));

    /* DMA Source Address Register (offset: 930h / 934h + channel offset) */
    l_data = (uint32_t) src_addr;
    /* Split the 64-bit source address into lower and upper 32-bit halves. */
    u_data = (uint32_t) (src_addr >> 32);
    /* Write the source address lower half to the DMA source address register. */
    *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x930U + dmac_ch_offset) = l_data;
    /* Write the source address upper half to the DMA source address register. */
    *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x934U + dmac_ch_offset) = u_data;

    /* DMA Destination Address Register (offset: 938h / 93Ch + channel offset) */
    l_data = (uint32_t) dest_addr;
    /* Split the 64-bit destination address into lower and upper 32-bit halves. */
    u_data = (uint32_t) (dest_addr >> 32);
    /* Write the destination address lower half to the DMA destination address register. */
    *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x938U + dmac_ch_offset) = l_data;
    /* Write the destination address upper half to the DMA destination address register. */
    *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x93CU + dmac_ch_offset) = u_data;

    /* DMA Size Register (offset: 928h + channel offset) */
    /* Write the transfer size to the DMA size register. */
    *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x928U + dmac_ch_offset) = write_buf;

    /* DMA Transaction Control Register (offset: 924h + channel offset) */
    if ((AXItoPCI) == (transfar_mode))
    {
        /* Configure the transfer direction as AXI to PCIe. */
        /* DMA_DIR[0] = 1(AXI->PCIe), CCH_D[19:16] = 0000 */
        *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x924U + dmac_ch_offset) = 0x00000001U;
    }
    else if ((PCItoAXI) == (transfar_mode))
    {
        /* Configure the transfer direction as PCIe to AXI. */
        /* DMA_DIR[0] = 0(PCIe->AXI), CCH_D[19:16] = 0001 */
        *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x924U + dmac_ch_offset) = 0x00010000U;
    }
    else
    {
        return FSP_ERR_UNSUPPORTED;
    }

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       Starts the PCIe RC DMA transfer on the specified channel and waits for the transfer to complete.
 * @param[in]   p_ctrl    Pointer to the PCIe RC control block
 * @param[in]   dmach     DMA channel number
 * @retval      FSP_SUCCESS             Upon successful completion of the transfer
 * @retval      FSP_ERR_UNSUPPORTED     Invalid DMA channel
 * @retval      FSP_ERR_ABORTED         The DMA transfer completed with an error
 **********************************************************************************************************************/
fsp_err_t R_PCIe_RC_DmacStart(pci_ctrl_t * const p_ctrl, uint8_t dmach)
{
    /* Get PCIe RC driver instance */
    pcie_rc_instance_ctrl_t * p_instance_ctrl = (pcie_rc_instance_ctrl_t *) p_ctrl;

    /* PCIe register base address */
    uint32_t pcie_base_address = 0;

    /* DMA channel register offset (0x80 bytes per channel) */
    uint32_t dmac_ch_offset = 0x80U * (uint32_t) dmach;

    static uint32_t rdata;
    static uint32_t mask;
    static uint32_t dma_int;

    pcie_base_address = R_PCIE0_BASE;

    if ((dmach) > (7))
    {
        return FSP_ERR_UNSUPPORTED; /* arg err */
    }

    /** Set RDMA_EN */
    /* DMAC Channel Control Register (offset: 900h + channel offset) */
    /* Trigger the register-method DMA transfer start. */
    /* Register method DMA transfer start */
    (*(volatile uint32_t *)(uintptr_t)(pcie_base_address + 0x900U + dmac_ch_offset)) |= (0x00000001U);
    /** Assert DMA_INT */
    /* Wait until the DMA interrupt table indicates the channel interrupt has been asserted. */
    do
    {
        dma_int = p_instance_ctrl->p_reg->PCI_RC_INTTABLE;
    } while ((0) == (dma_int & 0xff000000));

    /** Check the register */
    /* Read the DMA channel interrupt status register. */
    rdata = p_instance_ctrl->p_reg->PCI_RC_DMAINTS;

    /** Check CHn_END (n = dmach) */
    mask = (1UL << (4U * (uint32_t) dmach));
    if ((0) == (rdata & mask))
    {
        /** Check the register (for error analysis: optional) */
        /* DMAC Error Status Register (offset: 0x978 + channel offset) */
        /* Read the DMA error status register for diagnostic purposes. */
        rdata = *(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x978U + dmac_ch_offset);

        /** Proceed to error processing */
        PCIe_dma_errproc(p_ctrl, dmach);
        return FSP_ERR_ABORTED;
    }

    /** End of transfer */
    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief       Handles a PCIe RC DMA channel error by clearing the error interrupt flag and reading back the DMA
 *              status registers for diagnostic purposes.
 * @param[in]   p_ctrl    Pointer to the PCIe RC control block
 * @param[in]   dmach     DMA channel number
 * @retval      FSP_SUCCESS    Always returned upon completion
 **********************************************************************************************************************/
fsp_err_t PCIe_dma_errproc(pci_ctrl_t * const p_ctrl, uint8_t dmach)
{
    /* Get PCIe RC driver instance */
    pcie_rc_instance_ctrl_t * p_instance_ctrl = (pcie_rc_instance_ctrl_t *) p_ctrl;

    /* PCIe register base address */
    uint32_t pcie_base_address = R_PCIE0_BASE;

    /* DMA channel register offset (0x80 bytes per channel) */
    uint32_t dmac_ch_offset = 0x80U * (uint32_t) dmach;

    /* Clear DMA channel error interrupt flag */
    uint32_t shift;
    uint32_t mask;

    /* Calculate the bit shift for the DMA channel. */
    shift = 4U * (uint32_t) dmach;

    /* Create the interrupt clear mask. */
    mask = ~(UINT32_C(8) << shift);
    /* Clear the DMA interrupt status bit. */
    p_instance_ctrl->p_reg->PCI_RC_DMAINTS &= mask;

    /* Read DMA status registers for debug/progress check.
     * The values are intentionally discarded.
     */
    (void)*(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x950U + dmac_ch_offset);
    /* Read the DMA interrupt status register. */
    (void)*(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x960U + dmac_ch_offset);

    /* Read the DMA transfer status register. */
    (void)*(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x964U + dmac_ch_offset);

    /* Read the DMA source address register. */
    (void)*(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x968U + dmac_ch_offset);

    /* Read the DMA destination address register. */
    (void)*(volatile uint32_t *)((uintptr_t)pcie_base_address + 0x96CU + dmac_ch_offset);

    return FSP_SUCCESS;
}
