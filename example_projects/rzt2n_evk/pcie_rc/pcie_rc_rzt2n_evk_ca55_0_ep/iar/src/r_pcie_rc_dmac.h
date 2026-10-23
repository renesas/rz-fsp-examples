/***********************************************************************************************************************
 * File Name    : r_pcie_rc_dmac.h
 * Description  : PCIe RC DMA controller configuration and control function declarations and macros.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
*
* SPDX-License-Identifier: BSD-3-Clause
***********************************************************************************************************************/

#ifndef R_PCIE_RC_DMAC_H_
#define R_PCIE_RC_DMAC_H_

/******************************************************************************
 Macro definitions
 ******************************************************************************/
#define AXItoPCI                   (0)
#define PCItoAXI                   (1)

/******************************************************************************
 Exported global functions
 ******************************************************************************/
extern fsp_err_t R_PCIE_RC_DmacSet (
    pci_ctrl_t * const p_ctrl,
    uint8_t            dmach,
    uint32_t           transfar_mode,
    uint64_t           src_addr,
    uint64_t           dest_addr,
    uint32_t           write_buf);
extern fsp_err_t R_PCIe_RC_DmacStart(pci_ctrl_t * const p_ctrl, uint8_t dmach);
extern fsp_err_t PCIe_dma_errproc(pci_ctrl_t * const p_ctrl, uint8_t dmach);

#endif /* R_PCIE_RC_DMAC_H_ */
