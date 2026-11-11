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
#ifndef SSBL_H__
#define SSBL_H__

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "bsp_api.h"

/* Definition value file for each target device. */
#if defined(BSP_CFG_CORE_CR52)
 #if defined(BSP_MCU_GROUP_RZT2M)
  #include "target/rzt2m/ssbl_info_rzt2m.h"
 #elif defined(BSP_MCU_GROUP_RZT2L)
  #include "target/rzt2l/ssbl_info_rzt2l.h"
 #elif defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h/ssbl_info_rzt2h.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n/ssbl_info_rzt2n.h"
 #elif defined(BSP_MCU_GROUP_RZN2L)
  #include "target/rzn2l/ssbl_info_rzn2l.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h/ssbl_info_rzn2h.h"
 #endif
#elif defined(BSP_CFG_CORE_CA55)
 #if defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h_ca55/ssbl_info_rzt2h_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n_ca55/ssbl_info_rzt2n_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h_ca55/ssbl_info_rzn2h_ca55.h"
 #endif
#endif

/******************************************************************************
 * Exported global functions (to be accessed by other files)
 ******************************************************************************/
void second_application_boot_loader (void);

#endif /* SSBL_H__ */
