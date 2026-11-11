/***********************************************************************************************************************
 * File Name    : device_setup.c
 * Description  : Contains data structures and functions used in device_setup.c.
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
#include "device_setup.h"
#include "cmd_flash.h"
#include "cmd_otp.h"
#include "cmd_otp_auth.h"
#include "return_code.h"

/* Macro definitions */
/* USB related */
#define NUM_STRING_DESCRIPTOR   (7U)
#define LINE_CODING_LENGTH      (0x07U)
#define USB_READ_SIZE           (4096U)

/* Command payload length  */
#define PAYLOAD_WRITE_FLASH     (4U)
#define PAYLOAD_WRITE_OTP       (6U)
#define PAYLOAD_READ_OTP        (2U)
#define PAYLOAD_SET_JAUTH       (2U)
#define PAYLOAD_GET_JAUTH       (0U)
#define PAYLOAD_SET_JAUTHID     (18U)
#define PAYLOAD_SET_SCIUSB      (1U)
#define PAYLOAD_GET_SCIUSB      (0U)
#define PAYLOAD_GET_UID         (0U)
#define PAYLOAD_SET_SAUTH       (2U)
#define PAYLOAD_GET_SAUTH       (0U)
#define PAYLOAD_SET_SAUTHID     (17U)

/* The size of each value contained in the packet */
#define CMD_TYPE_SIZE           (1U)
#define CMD_CODE_SIZE           (1U)
#define PAYLOAD_SIZE            (4U)
#define CMD_HEADER_SIZE         (CMD_TYPE_SIZE + CMD_CODE_SIZE + PAYLOAD_SIZE + 2U)
#define FLASH_ADDRESS_SIZE      (4U)
#define OTP_ADDRESS_SIZE        (2U)
#define CHKSUM_SIZE             (1U)
#define RET_CODE_SIZE           (1U)
#define AUTH_MODE_SIZE          (1U)
#define BOOT_MODE_SIZE          (1U)
#define RECV_BYTE_SIZE          (1U)

/* Communication interface */
#define COMM_IF_SCI             (0U)
#define COMM_IF_USB             (1U)

/* SCI setting value  */
#define SCI_UART_BAUDRATE       (115200U)
#define SCI_BUND_RATE_ERR       (5000U)

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/
/* USB descriptor */
extern uint8_t g_apl_device[];
extern uint8_t g_apl_configuration[];
extern uint8_t g_apl_hs_configuration[];
extern uint8_t g_apl_qualifier_descriptor[];
extern uint8_t * g_apl_string_table[];

/* const */ usb_descriptor_t g_usb_descriptor =
{
    g_apl_device,                   /* Pointer to the device descriptor */
    g_apl_configuration,            /* Pointer to the configuration descriptor for Full-speed */
    g_apl_hs_configuration,         /* Pointer to the configuration descriptor for Hi-speed */
    g_apl_qualifier_descriptor,     /* Pointer to the qualifier descriptor */
    g_apl_string_table,             /* Pointer to the string descriptor table */
    NUM_STRING_DESCRIPTOR
};

/******************************************************************************
 * Private global variables and functions
 ******************************************************************************/
static volatile uint32_t s_g_sci_send_packet_complete     = 0U;  // Send packet completion flag 
static volatile uint32_t s_g_sci_receive_packet_complete  = 0U;  // Receive packet completion flag 
static uint32_t s_g_sci_payload_length                    = 0U;  // Receive payload size
static uint32_t s_g_sci_receive_length                    = 0U;  // Receive request packet size
static uint8_t  s_g_sci_receive_data                      = 0U;  // Receive packet data

static volatile uint32_t s_g_usb_receive_packet_complete  = 0U;  // Receive packet completion flag 
static uint32_t s_g_usb_receive_length                    = 0U;  // Receive request packet size

/* USB reassembly state */
static uint32_t s_g_usb_recv_offset       = 0U;  // Total bytes received so far
static uint32_t s_g_usb_expected_length   = 0U;  // Total packet length expected (0 = header not yet parsed)

static usb_pcdc_linecoding_t s_g_line_coding;

static uint32_t handle_command (uint8_t * const p_buf, uint32_t rcv_len);
static void send_response_packet (uint8_t * const p_buf, uint32_t send_len, uint32_t interface);
static void usb_write (uint8_t * const p_buf, uint32_t len);
static void usb_check_event (void);
static void sci_uart_set_baud (void);
static void sci_uart_write (uint8_t * const p_buf, uint32_t len);
static uint8_t calc_checksum (uint8_t * const p_data, uint32_t len);
static void handle_module_error (fsp_err_t fsp_err);
static void usb_read_request (void);

/******************************************************************************
 * @fn device_setup
 * @brief Device setup process.
 ******************************************************************************/
void device_setup(void)
{
    uint8_t * p_packet_buf = (uint8_t *)PACKET_BUFFER_ADDR;
    uint32_t  send_len     = 0U;
    fsp_err_t fsp_err;
    
    /* Initializes the module */
    fsp_err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    handle_module_error(fsp_err);
    sci_uart_set_baud();
    
    /* Additional processing to enable USB communication after USB boot */
    /* ---- Start ---- */
    /* Wait 1 second before USB reset */
    /* Depending on the timing, the reset may occur before the message transmission and */
    /* reception with the tool during USB boot is completed normally, so this is to prevent this */
    R_BSP_SoftwareDelay(1, BSP_DELAY_UNITS_SECONDS);
    
    /* Remove write protection */
    R_RWP_NS->PRCRN = (uint32_t)0x0000A507;
    R_RWP_S->PRCRS  = (uint32_t)0x0000A50F;
    
    /* Reset the USB module */
    uint32_t mrctle_val = R_SYSC_NS->MRCTLE;
    R_SYSC_NS->MRCTLE = mrctle_val | 0x00002000UL;
    
    /* Delay */
    R_BSP_SoftwareDelay(5, BSP_DELAY_UNITS_MICROSECONDS);
    
    R_SYSC_NS->MRCTLE = mrctle_val;
    /* ---- End ---- */
    
    fsp_err = R_USB_Open(&g_basic0_ctrl, &g_basic0_cfg);
    handle_module_error(fsp_err);
    
    fsp_err = R_CMTW_Open(&g_timer0_ctrl, &g_timer0_cfg);
    handle_module_error(fsp_err);    
    
    /* Start the timer */
    (void)R_CMTW_Start(&g_timer0_ctrl);
    (void)R_CMTW_Enable(&g_timer0_ctrl);
    
    /* First command read.(SCI UART) */
    /* The first USB read will be performed at the first event check */
    fsp_err = R_SCI_UART_Read(&g_uart0_ctrl, &s_g_sci_receive_data, RECV_BYTE_SIZE);
    handle_module_error(fsp_err);
    (void)R_CMTW_Reset(&g_timer0_ctrl);
    
    /* Receives device setup commands, processes commands, and sends results */
    while (1)
    {
        /* Check for USB event */
        usb_check_event();
        
        /* Check if SCI reception is complete */
        if (1U == s_g_sci_receive_packet_complete)
        {
            /* Execute command */
            send_len = handle_command(p_packet_buf, s_g_sci_receive_length);
            send_response_packet(p_packet_buf, send_len, COMM_IF_SCI);
        }
        
        /* Check if USB reception is complete */
        if (1U == s_g_usb_receive_packet_complete)
        {
            /* Execute command */
            send_len = handle_command(p_packet_buf, s_g_usb_receive_length);
            send_response_packet(p_packet_buf, send_len, COMM_IF_USB);
        }
    }
}

/******************************************************************************
 * @fn handle_command
 * @brief Command handler.
 *
 * @param[in,out]  p_buf          Packet buffer
 * @param[in]      rcv_len        Request packet length
 *
 * @return     Response packet length
 ******************************************************************************/
static uint32_t handle_command(uint8_t * const p_buf, uint32_t rcv_len)
{
    uint32_t  idx              = 0U;
    uint32_t  send_len         = 0U;
    uint32_t  write_data_size  = 0U;
    uint32_t  res_payload_len  = 0U;
    int32_t   cmp_ret          = 0;
    uint8_t   return_code      = 0U;
    uint8_t   auth_mode        = 0U;
    uint8_t   uid[8]           = {0};
    uint8_t   boot_mode        = 0U;
    uint8_t   sum              = 0U;
    otp_data_width_t otp_data  = 0U;
    uint8_t    cmd_code        = CODE_UNDEF;
    packet_t * p_packet        = (packet_t *)p_buf;
    
    /* Checksum verification */
    sum = calc_checksum(p_buf, rcv_len - 1U);
    cmp_ret = memcmp(&p_buf[rcv_len - CHKSUM_SIZE], (uint8_t *)&sum, CHKSUM_SIZE);
    if (0 != cmp_ret)
    {
        res_payload_len = RET_CODE_SIZE;
        return_code     = RET_DATA_FAIL;
    }
    else
    {
        /* Handle command */
        cmd_code = p_packet->head.code;
        switch (cmd_code)
        {
            /* WRITE_FLASH command */
            case CODE_WRITE_FLASH:
                res_payload_len = RET_CODE_SIZE;
                write_data_size = p_packet->head.payload_size - FLASH_ADDRESS_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_WRITE_FLASH + CHKSUM_SIZE) <= rcv_len)
                {
                    /* Write to the flash */
                    return_code = cmd_write_flash(p_packet->cmd.wflash.address, p_packet->cmd.wflash.data, write_data_size);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* WRITE_OTP command */
            case CODE_WRITE_OTP:
                res_payload_len = RET_CODE_SIZE;
                memcpy(&otp_data, p_packet->cmd.wotp.data, sizeof(otp_data_width_t));
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_WRITE_OTP + CHKSUM_SIZE) == rcv_len)
                {
                    /* Write to the otp */
                    return_code = cmd_write_otp(p_packet->cmd.wotp.address, otp_data);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* READ_OTP command */
            case CODE_READ_OTP:
                res_payload_len = RET_CODE_SIZE + sizeof(otp_data_width_t);
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_READ_OTP + CHKSUM_SIZE) == rcv_len)
                {
                    /* Read OTP */
                    return_code = cmd_read_otp(p_packet->cmd.rotp.address, &otp_data);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
            
            /* SET_JTAG_AUTH command */
            case CODE_SET_JTAG_AUTH:
                res_payload_len = RET_CODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_SET_JAUTH + CHKSUM_SIZE) == rcv_len)
                {
                    /* Setup JTAG authentication */
                    return_code = cmd_set_jtag_auth(p_packet->cmd.jauth.mode, p_packet->cmd.jauth.type);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* GET_JTAG_AUTH command */
            case CODE_GET_JTAG_AUTH:
                res_payload_len = RET_CODE_SIZE + AUTH_MODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_GET_JAUTH + CHKSUM_SIZE) == rcv_len)
                {
                    /* Get JTAG authentication information */
                    return_code = cmd_get_jtag_auth(&auth_mode);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* SET_JTAG_AUTHID command */
            case CODE_SET_JTAG_AUTHID:
                res_payload_len = RET_CODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_SET_JAUTHID + CHKSUM_SIZE) == rcv_len)
                {
                    /* Setup JTAG authentication ID */
                    return_code = cmd_set_jtag_auth_id(p_packet->cmd.jauthid.mode, p_packet->cmd.jauthid.type, p_packet->cmd.jauthid.id);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
#if defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L)
            /* SET_SCIUSB command */
            case CODE_SET_SCIUSB:
                res_payload_len = RET_CODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_SET_SCIUSB + CHKSUM_SIZE) == rcv_len)
                {
                    /* Setup SCI/USB boot */
                    return_code = cmd_set_sci_usb_boot(p_packet->cmd.sciusb.mode);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* GET_SCIUSB command */
            case CODE_GET_SCIUSB:
                res_payload_len = RET_CODE_SIZE + BOOT_MODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_GET_SCIUSB + CHKSUM_SIZE) == rcv_len)
                {
                    /* Get SCI/USB boot information */
                    return_code = cmd_get_sci_usb_boot(&boot_mode);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
#endif /* defined(BSP_MCU_GROUP_RZT2M) || defined(BSP_MCU_GROUP_RZT2L) || defined(BSP_MCU_GROUP_RZN2L) */
            /* GET_UID command */
            case CODE_GET_UID:
                res_payload_len = RET_CODE_SIZE + UID_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_GET_UID + CHKSUM_SIZE) == rcv_len)
                {
                    /* Get unique ID */
                    return_code = cmd_get_unique_id(uid);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
#if defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H)
            /* SET_SCIUSB_AUTH command */
            case CODE_SET_SCIUSB_AUTH:
                res_payload_len = RET_CODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_SET_SAUTH + CHKSUM_SIZE) == rcv_len)
                {
                    /* Setup SCI/USB boot authentication */
                    return_code = cmd_set_sci_usb_boot_auth(p_packet->cmd.sauth.mode, p_packet->cmd.sauth.type);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* GET_SCIUSB_AUTH command */
            case CODE_GET_SCIUSB_AUTH:
                res_payload_len = RET_CODE_SIZE + AUTH_MODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_GET_SAUTH + CHKSUM_SIZE) == rcv_len)
                {
                    /* Get SCI/USB boot authentication information */
                    return_code = cmd_get_sci_usb_boot_auth(&auth_mode);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
            /* SET_SCIUSB_AUTHID command */
            case CODE_SET_SCIUSB_AUTHID:
                res_payload_len = RET_CODE_SIZE;
                
                /* Check the received Response packet length */
                if ((CMD_HEADER_SIZE + PAYLOAD_SET_SAUTHID + CHKSUM_SIZE) == rcv_len)
                {
                    /* Setup SCI/USB boot authentication ID */
                    return_code = cmd_set_sci_usb_boot_auth_id(p_packet->cmd.sauthid.type, p_packet->cmd.sauthid.id);
                }
                else
                {
                    return_code = RET_DATA_FAIL;
                }
                
                break;
                
#endif /* defined(BSP_MCU_GROUP_RZT2H) || defined(BSP_MCU_GROUP_RZT2N) || defined(BSP_MCU_GROUP_RZN2H) */
            /* Unknown command */
            default:
                cmd_code        = CODE_UNDEF;
                res_payload_len = RET_CODE_SIZE;
                return_code     = RET_DATA_FAIL;
                break;
        }
    }
    
    /* Create the header part of the response packet */
    /* Clears buffer header information */
    memset(p_packet, 0x00U, sizeof(head_t));
    
    /* Add command type */
    p_packet->head.type = TYPE_RES;
    
    /* Add command code */
    p_packet->head.code = cmd_code;
    
    /* Add payload size */
    p_packet->head.payload_size = res_payload_len;
    idx = sizeof(head_t);
    
    /* Create the payload part of the response packet */
    /* Add return code */
    p_buf[idx] = return_code;
    idx += RET_CODE_SIZE;
    
    /* Add read data */
    switch (cmd_code)
    {
        case CODE_READ_OTP:
            memcpy(&p_buf[idx], (uint8_t *)&otp_data, sizeof(otp_data_width_t));
            idx += sizeof(otp_data_width_t);
            break;
            
        case CODE_GET_JTAG_AUTH:
        case CODE_GET_SCIUSB_AUTH:
            memcpy(&p_buf[idx], &auth_mode, AUTH_MODE_SIZE);
            idx += AUTH_MODE_SIZE;
            break;
            
        case CODE_GET_SCIUSB:
            memcpy(&p_buf[idx], &boot_mode, BOOT_MODE_SIZE);
            idx += BOOT_MODE_SIZE;
            break;
            
        case CODE_GET_UID:
            memcpy(&p_buf[idx], uid, UID_SIZE);
            idx += UID_SIZE;
            break;
            
        default:
            break;
    }
    
    /* Add checksum */
    p_buf[idx] = calc_checksum(p_buf, idx);
    send_len   = idx + CHKSUM_SIZE;
    
    return send_len;
}

/******************************************************************************
 * @fn send_response_packet
 * @brief Send response packet to the tool. 
 *
 * @param[in]  p_buf          Packet buffer
 * @param[in]  send_len       Response packet length
 * @param[in]  interface      Interface
 ******************************************************************************/
static void send_response_packet(uint8_t * const p_buf, uint32_t send_len, uint32_t interface)
{    
    /* Send Response packet.*/
    if (COMM_IF_SCI == interface)
    {
        /* SCI */
        sci_uart_write(p_buf, send_len);
    }
    else if (COMM_IF_USB == interface)
    {
        /* USB */
        usb_write(p_buf, send_len);
    }
    else
    {
        /* Unsupported interface */
    }
}

/******************************************************************************
 * @fn usb_write
 * @brief Send data by USB communication.
 *
 * @param[in]  p_buf          Packet buffer
 * @param[in]  len            Receive data length
 ******************************************************************************/
static void usb_write(uint8_t * const p_buf, uint32_t len)
{
    fsp_err_t fsp_err;
    
    fsp_err = R_USB_Write(&g_basic0_ctrl, p_buf, len, USB_CLASS_PCDC);
    handle_module_error(fsp_err);
    
    s_g_usb_receive_packet_complete = 0U;
    s_g_usb_receive_length          = 0U;
    s_g_usb_recv_offset             = 0U;   /* <-- reset reassembly */
    s_g_usb_expected_length         = 0U;   /* <-- reset reassembly */
}

/******************************************************************************
 * @fn usb_read_request
 * @brief Issue a USB read for the next chunk (max USB_READ_SIZE bytes)
 *        into the packet buffer at the current receive offset.
 ******************************************************************************/
static void usb_read_request(void)
{
    uint8_t * p_packet_buf = (uint8_t *)PACKET_BUFFER_ADDR;
    fsp_err_t fsp_err;

    fsp_err = R_USB_Read(&g_basic0_ctrl,
                         &p_packet_buf[s_g_usb_recv_offset],
                         USB_READ_SIZE,
                         USB_CLASS_PCDC);
    handle_module_error(fsp_err);
}

/******************************************************************************
 * @fn usb_check_event
 * @brief USB event check.
 ******************************************************************************/
static void usb_check_event(void)
{
    uint8_t        * p_packet_buf = (uint8_t *)PACKET_BUFFER_ADDR;
    usb_event_info_t event_info;
    usb_status_t     event;
    
    /* Get USB event data */
    R_USB_EventGet(&event_info, &event);
    
    /* Handle the received event (if any) */
    switch (event)
    {
        case USB_STATUS_CONFIGURED:
        case USB_STATUS_WRITE_COMPLETE:
            /* Start receiving a new packet from offset 0 */
            s_g_usb_recv_offset     = 0U;
            s_g_usb_expected_length = 0U;
            usb_read_request();
            break;
            
        case USB_STATUS_READ_COMPLETE:
            /* Accumulate the received chunk */
            s_g_usb_recv_offset += event_info.data_size;

            /* Parse header once enough bytes have arrived */
            if ((0U == s_g_usb_expected_length) &&
                (s_g_usb_recv_offset >= CMD_HEADER_SIZE))
            {
                packet_t * p_packet = (packet_t *)p_packet_buf;
                s_g_usb_expected_length =
                    CMD_HEADER_SIZE + p_packet->head.payload_size + CHKSUM_SIZE;
            }

            /* Check whether the full packet has been received */
            if ((0U != s_g_usb_expected_length) &&
                (s_g_usb_recv_offset >= s_g_usb_expected_length))
            {
                s_g_usb_receive_length          = s_g_usb_expected_length;
                s_g_usb_receive_packet_complete = 1U;
            }
            else
            {
                /* Need more data: issue another read for the next chunk */
                usb_read_request();
            }
            break;
            
        case USB_STATUS_REQUEST:   /* Receive Class Request */
            if (USB_PCDC_SET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
            {
                /* Configure virtual UART settings */
                R_USB_PeriControlDataGet(&g_basic0_ctrl, (uint8_t *) &s_g_line_coding, LINE_CODING_LENGTH);
            }
            else if (USB_PCDC_GET_LINE_CODING == (event_info.setup.request_type & USB_BREQUEST))
            {
                /* Send virtual UART settings back to host */
                R_USB_PeriControlDataSet(&g_basic0_ctrl, (uint8_t *) &s_g_line_coding, LINE_CODING_LENGTH);
            }
            else
            {
                /* ACK all other status requests */
                R_USB_PeriControlStatusSet(&g_basic0_ctrl, USB_SETUP_STATUS_ACK);
            }
            break;
            
        case USB_STATUS_SUSPEND:
        case USB_STATUS_DETACH:
            break;
            
        default:
            break;
    }
}

/******************************************************************************
 * @fn sci_uart_set_baud
 * @brief Set the band for SCI communication.
 ******************************************************************************/
static void sci_uart_set_baud(void)
{
    sci_baud_setting_t          baud_setting;
    sci_uart_baud_calculation_t uart_band_calc;
    fsp_err_t                   fsp_err;
    
    uart_band_calc.baud_rate_error_x_1000 = SCI_BUND_RATE_ERR;
    uart_band_calc.baudrate               = SCI_UART_BAUDRATE;
    uart_band_calc.bitrate_modulation     = false;
    
    fsp_err = R_SCI_UART_BaudCalculate(&uart_band_calc, SCI_UART_CLOCK_SOURCE_SCI0ASYNCCLK, &baud_setting);
    handle_module_error(fsp_err);
    fsp_err = R_SCI_UART_BaudSet(&g_uart0_ctrl, (void *)&baud_setting);
    handle_module_error(fsp_err);
}

/******************************************************************************
 * @fn sci_uart_write
 * @brief Send data by SCI communication.
 *
 * @param[in]  p_buf          Packet buffer
 * @param[in]  len            Receive data length
 ******************************************************************************/
static void sci_uart_write(uint8_t * const p_buf, uint32_t len)
{
    fsp_err_t fsp_err;
    uint32_t status;
    
    /* Send data */
    fsp_err = R_SCI_UART_Write(&g_uart0_ctrl, p_buf, len);
    handle_module_error(fsp_err);
    
    /* Wait for transmission */
    do
    {
        status = s_g_sci_send_packet_complete;
    }
    while (!status);
    
    /* Clear flags */
    s_g_sci_send_packet_complete = 0U;
}

/******************************************************************************
 * @fn calc_checksum
 * @brief Checksum calculation. (1byte)
 *
 * @param[in]  p_data         Target data
 * @param[in]  len            Data length
 *
 * @return     Checksum value
 ******************************************************************************/
static uint8_t calc_checksum(uint8_t * const p_data, uint32_t len)
{
    uint8_t    sum = 0U; // Checksum value
    uint32_t   cnt = 0U; // Byte counter
    
    for (cnt = 0U; cnt < len; cnt++)
    {
        sum = (uint8_t)(sum + *(p_data + cnt));
    }
    
    return sum;
}

/******************************************************************************
 * @fn handle_module_error
 * @brief Module error handler.
 *
 * @param[in]  fsp_err        FSP module return code.
 ******************************************************************************/
static void handle_module_error(fsp_err_t fsp_err)
{
    /* If an error occurs in each module, the setup process is stopped */
    if (FSP_SUCCESS != fsp_err)
    {
        while (1);
    }
}

/******************************************************************************
 * @fn sci_uart_callback
 * @brief SCI UART module callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void sci_uart_callback(uart_callback_args_t * p_args)
{
    fsp_err_t fsp_err;
    uint8_t * p_packet_buf = (uint8_t *)PACKET_BUFFER_ADDR;
    
    /* Handle the UART event */
    switch (p_args->event)
    {
        /* Receive complete */
        case UART_EVENT_RX_COMPLETE:
            (void)R_CMTW_Reset(&g_timer0_ctrl);
            p_packet_buf[s_g_sci_receive_length] = s_g_sci_receive_data;
            s_g_sci_receive_length++;
            
            if (CMD_HEADER_SIZE == s_g_sci_receive_length)
            {
                /* Check command type */
                if (TYPE_REQ != ((packet_t *)p_packet_buf)->head.type)
                {
                    s_g_sci_receive_length = 0U;
                }
                
                /* Store payload_size */
                s_g_sci_payload_length = ((packet_t *)p_packet_buf)->head.payload_size;
            }
            else if ((CMD_HEADER_SIZE + s_g_sci_payload_length + CHKSUM_SIZE) == s_g_sci_receive_length)
            {
                /* Stop the timer */
                (void)R_CMTW_Disable(&g_timer0_ctrl);
                (void)R_CMTW_Stop(&g_timer0_ctrl);
                
                s_g_sci_receive_packet_complete = 1U;
                break;
            }
            else
            {
                /* Do nothing */
            }
            
            fsp_err = R_SCI_UART_Read(&g_uart0_ctrl, &s_g_sci_receive_data, RECV_BYTE_SIZE);
            handle_module_error(fsp_err);
            
            break;
            
        /* Transmit complete */
        case UART_EVENT_TX_COMPLETE:
            s_g_sci_send_packet_complete = 1U;
            
            /* Start the timer */
            (void)R_CMTW_Start(&g_timer0_ctrl);
            (void)R_CMTW_Enable(&g_timer0_ctrl);
            
            /* Receive command header. (SCI UART) */
            fsp_err = R_SCI_UART_Read(&g_uart0_ctrl, &s_g_sci_receive_data, RECV_BYTE_SIZE);
            handle_module_error(fsp_err);
            
            (void)R_CMTW_Reset(&g_timer0_ctrl);
            
            /* Clear flags */
            s_g_sci_receive_length          = 0U;
            s_g_sci_payload_length          = 0U;
            s_g_sci_receive_packet_complete = 0U;
            
            break;
            
        default:
            break;
    }
}

/******************************************************************************
 * @fn cmtw_timer_callback
 * @brief CMTW TIMER module callback function.
 *
 * @param[in]  p_args         Callback information.
 ******************************************************************************/
void cmtw_timer_callback(timer_callback_args_t * p_args)
{
    /* Handle the TIMER event */
    switch (p_args->event)
    {
        /* Cycle end event */
        case TIMER_EVENT_CYCLE_END:
            s_g_sci_receive_length          = 0U;
            s_g_sci_payload_length          = 0U;
            s_g_sci_receive_packet_complete = 0U;
            break;
            
        default:
            break;
    }
}
