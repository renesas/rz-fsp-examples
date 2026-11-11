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
#ifndef FWUPDATE_H__
#define FWUPDATE_H__

/******************************************************************************
 * Includes   <System Includes> , "Project Includes"
 ******************************************************************************/
#include "bsp_api.h"
#include "fwupdate_thread.h"

/* Definition value file for each target device. */
#if defined(BSP_CFG_CORE_CR52)
 #if defined(BSP_MCU_GROUP_RZT2M)
  #include "target/rzt2m/fwupdate_info_rzt2m.h"
 #elif defined(BSP_MCU_GROUP_RZT2L)
  #include "target/rzt2l/fwupdate_info_rzt2l.h"
 #elif defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h/fwupdate_info_rzt2h.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n/fwupdate_info_rzt2n.h"
 #elif defined(BSP_MCU_GROUP_RZN2L)
  #include "target/rzn2l/fwupdate_info_rzn2l.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h/fwupdate_info_rzn2h.h"
 #endif
#elif defined(BSP_CFG_CORE_CA55)
 #if defined(BSP_MCU_GROUP_RZT2H)
  #include "target/rzt2h_ca55/fwupdate_info_rzt2h_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZT2N)
  #include "target/rzt2n_ca55/fwupdate_info_rzt2n_ca55.h"
 #elif defined(BSP_MCU_GROUP_RZN2H)
  #include "target/rzn2h_ca55/fwupdate_info_rzn2h_ca55.h"
 #endif
#endif

/******************************************************************************
 * Macro definitions
 ******************************************************************************/
#define INADDR_ANY                               (0x00000000)

/* Command code */
#define CMD_CODE_NONE                            ((uint8_t)0x00U)
#define CMD_CODE_START_UPDATE                    ((uint8_t)0x11U)
#define CMD_CODE_FIRMWARE_DATA                   ((uint8_t)0x12U)
#define CMD_CODE_ACK                             ((uint8_t)0x81U)
#define CMD_CODE_NACK                            ((uint8_t)0x82U)

/* NACK packet error code */
#define NACK_CODE_NONE                           ((uint8_t)0x00U)
#define NACK_CODE_FILE_SIZE_EXCEED               ((uint8_t)0x01U)
#define NACK_CODE_FILE_INVALID                   ((uint8_t)0x02U)
#define NACK_CODE_USER_APPLICATION_WRITE_FAIL    ((uint8_t)0x03U)
#define NACK_CODE_PARAMETER_WRITE_FAIL           ((uint8_t)0x04U)
#define NACK_CODE_FLASH_MNG_AREA_UPDATE_FAIL     ((uint8_t)0x06U)

/******************************************************************************
 * Typedef definitions
 ******************************************************************************/
/* Update processing status */
typedef enum e_update_proc_status
{
    IDLE = 0,
    UPDATING,
} update_proc_status_t;

/* Control packet format */
typedef struct st_packet_header_t
{
    uint8_t  command;
    uint8_t  dummy[3];
    uint32_t payload_len;
    uint8_t  payload[0];
} packet_header_t;

/******************************************************************************
 * External global variables
 ******************************************************************************/
extern volatile update_proc_status_t g_update_status_udp;
extern volatile update_proc_status_t g_update_status_tcp;
extern          TaskHandle_t         fwupdate_thread;

/******************************************************************************
 * Exported global functions
 ******************************************************************************/
void packet_handler (void);
void create_send_packet (packet_header_t * const p_rcv_packet, uint8_t cmd_code, uint8_t err_code);

#endif /* FWUPDATE_H__ */
