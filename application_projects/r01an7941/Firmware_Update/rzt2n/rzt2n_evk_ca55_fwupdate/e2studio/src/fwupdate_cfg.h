/***********************************************************************************************************************
 * Copyright [2020-2025] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics Corporation and/or its affiliates and may only
 * be used with products of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.
 * Renesas products are sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for
 * the selection and use of Renesas products and Renesas assumes no liability.  No license, express or implied, to any
 * intellectual property right is granted by Renesas.  This software is protected under all applicable laws, including
 * copyright laws. Renesas reserves the right to change or discontinue this software and/or this documentation.
 * THE SOFTWARE AND DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND
 * TO THE FULLEST EXTENT PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 * INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE
 * SOFTWARE OR DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.
 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR
 * DOCUMENTATION (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 * INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY
 * LOST PROFITS, OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/
#ifndef FWUPDATE_CFG_H__
#define FWUPDATE_CFG_H__

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
/** Boot mode selection
 *  @note  Set the boot mode value to use to [FWUPDATE_CFG_BOOT_MODE_SELECT].
 *         An external flash corresponding to the specified boot mode can be used.
 * 
 *  @value BOOT_MODE_XSPI0 : xSPI0 boot mode.
 *                           Use the XSPI0 flash area in the following address range.
 *                           The address varies by device.
 *                           RZ/T2M,T2L,N2L : 0x60000000 - 0x63FFFFFF (64Mbyte)
 *                           RZ/T2H,T2N,N2H : 0x40000000 - 0x43FFFFFF (64Mbyte)
 *         BOOT_MODE_XSPI1 : xSPI1 boot mode.
 *                           Use the XSPI1 flash area in the following address range.
 *                           The address varies by device.
 *                           RZ/T2L          : 0x68000000 - 0x68FFFFFF (16Mbyte)
 *                           RZ/T2H,T2N, N2H : 0x50000000 - 0x50FFFFFF (16Mbyte)
 *         BOOT_MODE_NOR   : Bus boot mode.
 *                           Use the NOR flash area in the following address range.
 *                           0x70000000 - 0x71FFFFFF (32Mbyte)
 */
#define FWUPDATE_CFG_BOOT_MODE_SELECT    (BOOT_MODE_XSPI0)
#define BOOT_MODE_XSPI0                  (0U)
#define BOOT_MODE_XSPI1                  (1U)
#define BOOT_MODE_NOR                    (2U)

/** Flash area management selection
 *  @note  Set the value for any flash area management to [FWUPDATE_CFG_FLASH_MNG_AREA].
 * 
 *  @value FLASH_MNG_AREA_SINGLE_BANK : Use flash on single bank.
 *                                      The previous application (before update) is not retained.
 *         FLASH_MNG_AREA_DUAL_BANK   : Use flash on dual bank.
 *                                      The previous application (before update) is retained.
 */
#define FWUPDATE_CFG_FLASH_MNG_AREA      (FLASH_MNG_AREA_SINGLE_BANK)
#define FLASH_MNG_AREA_SINGLE_BANK       (1U)
#define FLASH_MNG_AREA_DUAL_BANK         (2U)
#endif /* FWUPDATE_CFG_H__ */
