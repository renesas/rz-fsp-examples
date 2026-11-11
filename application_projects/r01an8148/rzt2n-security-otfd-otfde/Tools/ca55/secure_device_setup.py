########################################################################################################################
# DISCLAIMER
# This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
# other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
# applicable laws, including copyright laws.
# THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
# THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
# EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
# SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
# SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
# Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
# this software. By using this software, you agree to the additional terms and conditions found by accessing the
# following link:
# http://www.renesas.com/disclaimer
#
# Copyright (C) 2022-2025 Renesas Electronics Corporation. All rights reserved.
########################################################################################################################
# secure_device_setup.py : Tool Version 3.00-t2n-beta
import os
import re
import argparse
import time
import serial
from serial.tools import list_ports

# Log output settings when the "start" command is executed
# Default is "0" and log is output.
START_CMD_LOG_DISABLE = 0

# Message print
ERRMSG = print
RESMSG = print

# Error Message
ERRMSG_OPEN_FAIL        = __file__ + ' : error: No such file : %s'
ERRMSG_FILE_SIZE_FAIL   = __file__ + ' : error: The file size is invalid : %s'
ERRMSG_ADDR_FAIL        = __file__ + ' : error: The input value of the address is invalid : %s'
ERRMSG_DATA_FAIL        = __file__ + ' : error: The input value of the data is invalid : %s'
ERRMSG_PORTFAIL         = __file__ + ' : error: The specified COM port is invalid : %s'
ERRMSG_RCVCMDFAIL       = __file__ + ' : error: The command reception is invalid'

# Result Message
RESMSG_SUCCESS     = '%s : Setup success.'
RESMSG_DATA_FAIL   = '%s : Setup failure. Data is invalid.'
RESMSG_WRITE_FAIL  = '%s : Setup failure. Data write failure.'
RESMSG_READ_FAIL   = '%s : Setup failure. Data read failure.'
RESMSG_CONT_SEND   = 'Continue sending split data. Sent size %s / %s Byte.'

RETMSG_START_END   = 'Program send completed.'
RETMSG_START_FAIL  = 'Failed to send the program file.'

# S0 HDR
S0_OPEN      = 'S00700004F50454EC6\r\n'
S0_HDR_NM    = 'S0090000484452204E4D5D\r\n'
S0_HDR_HA    = 'S00900004844522048416F\r\n'

# SCI/USB boot type
BOOT_NORMAL = 'normal'
BOOT_AUTH   = 'auth'

# Send SO HDR data
s0_hdr_msg = {
    BOOT_NORMAL:  S0_HDR_NM,  # HDR NM
    BOOT_AUTH  :  S0_HDR_HA,  # HDR HA
}

# Serial setting
BOOT_TIMEOUT   = 10       # Timeout value (sec) for SCI/USB boot
BOOT_BAUDRATE  = 115200   # Baudrate for SCI/USB boot
SETUP_TIMEOUT  = 30       # Timeout value (sec) for Device Setup
SETUP_BAUDRATE = 115200   # Baudrate for Device Setup

# Size
WORD_BYTES            = 4
HWORD_BYTES           = WORD_BYTES // 2
BYTE_MAX_NUM          = 255
PASS_INFO_LEN         = 84
ROOT_PKEY_HASH_LEN    = 32
PASSWORD_LEN          = 16

# Flash write parameters
XSPI_FLASH_SECTOR_LEN = 0x00001000   # xSPI flash sector unit
NOR_FLASH_SECTOR_LEN  = 0x00020000   # NOR flash sector unit
SETUP_SEND_DATA_MAX   = 0x000C0000   # Maximum size at the time of transmission (768 KB)

# Address info
XSPI_START_ADDR       = 0x60000000   # xSPI area start address
NOR_START_ADDR        = 0x70000000   # NOR area start address
FLASH_ADDR_MASK       = 0xF0000000   # Flash address mask value

# Packet data 2 bytes Dummy area
DUMMY_AREA_2BYTE_VAL  = b'\x00\x00'

# Command type
TYPE_REQ              = b'\x50'
TYPE_RES              = b'\x55'

# Command code
CODE_WRITE_FLASH       = b'\x11'
CODE_WRITE_OTP         = b'\x12'
CODE_READ_OTP          = b'\x13'
CODE_SET_JTAG_AUTH     = b'\x14'
CODE_GET_JTAG_AUTH     = b'\x15'
CODE_SET_JTAG_AUTHID   = b'\x16'
CODE_SET_SCIUSB        = b'\x17'
CODE_GET_SCIUSB        = b'\x18'
CODE_INJECT_KEY        = b'\x1a'
CODE_SET_BOOT          = b'\x1b'
CODE_GET_BOOT          = b'\x1c'
CODE_GET_UID           = b'\x21'
CODE_SET_SCIUSB_AUTH   = b'\x22'
CODE_GET_SCIUSB_AUTH   = b'\x23'
CODE_SET_SCIUSB_AUTHID = b'\x24'
CODE_INJECT_CERT       = b'\x25'

# Payload size (Excluding variable values)
SIZE_WRITE_FLASH       = 4
SIZE_WRITE_OTP         = 6
SIZE_READ_OTP          = 2
SIZE_SET_JTAG_AUTH     = 2
SIZE_SET_JTAG_AUTHID   = 86
SIZE_SET_SCIUSB        = 1
SIZE_SET_SCIUSB_AUTH   = 2
SIZE_SET_SCIUSB_AUTHID = 85
SIZE_INJECT_KEY        = 0
SIZE_SET_BOOT          = 33
SIZE_INJECT_CERT       = 0
SIZE_NO_PAYLOAD        = 0

# JTAG authentication mode
JTAG_AUTH_MODE_AUTHLV1  = 1
JTAG_AUTH_MODE_AUTHLV2  = 2
JTAG_AUTH_MODE_PROHIBIT = 4

# SCI/USB boot authentication mode
SCI_USB_BOOT_AUTH_MODE_AUTH     = 1
SCI_USB_BOOT_AUTH_MODE_PROHIBIT = 2

# SCI/USB boot mode
SCI_USB_BOOT_MODE_ENABLE  = 0
SCI_USB_BOOT_MODE_DISABLE = 1

# Authentication type
AUTH_TYPE_PLAIN           = 0
AUTH_TYPE_HASH            = 1

# Setting flag
SEC_BOOT_SET_FLAG_OFF     = 0
SEC_BOOT_SET_FLAG_ON      = 1

# Return code
RET_SUCCESS     =  0   # b'\x00' Setup success.
RET_DATA_FAIL   = 17   # b'\x11' Setup failure. Data is invalid.
RET_WRITE_FAIL  = 18   # b'\x12' Setup failure. Data write failure.
RET_READ_FAIL   = 19   # b'\x13' Setup failure. Data read failure.

# Endian string
ENDIAN_BIG      = 'big'
ENDIAN_LITTLE   = 'little'

def main():
    """Main processing.
    
    Parsing command line arguments.
    Initial setup of device. 
    
    
    Args:
        None
    
    Returns:
        None
    
    """
    # Command line argument perse.
    parser = set_cmdline_argparser()
    args = parser.parse_args()
    
    # Check the COM port specification used for command transmission.
    ports = list_ports.comports()
    devices = [info.device for info in ports]
    if args.port not in devices:
        ERRMSG(str.format(ERRMSG_PORTFAIL % (args.port)))
        exit(1)
    
    # Initial setup of device.
    # (args.cmd is always set to some setup command)
    if args.cmd == 'writeflash':
        # Check whether required options are specified.
        if re.match("^[0-9A-Fa-f]{8}$", args.addr) is None:
            ERRMSG(str.format(ERRMSG_ADDR_FAIL % (args.addr)))
            exit(1)
        
        write_addr = int(args.addr, 16)
        flash_addr = write_addr & FLASH_ADDR_MASK
        
        # Read the data in the file.
        data = read_bin_file(args.i)
        if data is None:
            exit(1)
        
        write_comp_len = 0
        
        # Check sector size.
        if XSPI_START_ADDR == flash_addr:
            sector_size = XSPI_FLASH_SECTOR_LEN
            
        else:
            sector_size = NOR_FLASH_SECTOR_LEN
        
        # Check if data needs to be split.
        if SETUP_SEND_DATA_MAX < len(data):
            write_len = sector_size - (write_addr % sector_size)
            last_data_flg = False
            
        else:
            write_len = len(data)
            last_data_flg = True
        
        while True:
            # Set write data.
            write_data = data[write_comp_len:write_comp_len + write_len]
            
            # Write data to flash.
            send_buf = write_flash(write_addr + write_comp_len, write_data)
            
            # Check flag.
            if last_data_flg is True:
                break
            
            # Sending and receiving commands to and from the device.
            rcv_buf = setup_by_serial_comm(args.port, send_buf)
            if rcv_buf is None:
                ERRMSG(ERRMSG_RCVCMDFAIL)
                exit(1)
            
            write_comp_len += write_len
            
            # Check the received response data.
            ret = check_intermediate_respons_data(rcv_buf, send_buf[1], args.cmd, write_comp_len, len(data))
            if ret == -1:
                exit(1)
            
            # Set the write data size.
            if SETUP_SEND_DATA_MAX > (len(data) - write_comp_len):
                write_len = len(data) - write_comp_len
                last_data_flg = True
            
            else:
                write_len = SETUP_SEND_DATA_MAX
        
    elif args.cmd == 'writeotp':
        # Check whether required options are specified.
        if re.match("^[0-9A-Fa-f]{4}$", args.addr) is None:
            ERRMSG(str.format(ERRMSG_ADDR_FAIL % (args.addr)))
            exit(1)
        
        if re.match("^[0-9A-Fa-f]{4,8}$", args.data) is None:
            ERRMSG(str.format(ERRMSG_DATA_FAIL % (args.data)))
            exit(1)
        
        write_addr = int(args.addr, 16)
        write_data = int(args.data, 16)
        
        # Write data to otp.
        send_buf = write_otp(write_addr, write_data)
        
    elif args.cmd == 'readotp':
        # Check whether required options are specified.
        if re.match("^[0-9A-Fa-f]{4}$", args.addr) is None:
            ERRMSG(str.format(ERRMSG_ADDR_FAIL % (args.addr)))
            exit(1)
        
        read_addr = int(args.addr, 16)
        
        # Read otp data.
        send_buf = read_otp(read_addr)
        
    elif args.cmd == 'setjauth':
        # Setup JTAG authentication.
        send_buf = set_jauth(args.mode)
        
    elif args.cmd == 'getjauth':
        # Get JTAG authentication info.
        send_buf = msg_without_payload(CODE_GET_JTAG_AUTH)
        
    elif args.cmd == 'setjauthid':
        # Read the data in the file.
        data = read_bin_file(args.i)
        if data is None:
            exit(1)
        
        # Check whether required options are specified.
        if PASS_INFO_LEN != len(data):
            ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.i)))
            exit(1)
        
        # Setup JTAG authentication ID.
        send_buf = set_jauthid(args.mode, data)
        
    elif args.cmd == 'setsciboot' or args.cmd == 'setusbboot':
        # Setup SCI/USB boot disabled.
        send_buf = set_sciusb()
        
    elif args.cmd == 'getsciboot' or args.cmd == 'getusbboot':
        # Get SCI/USB boot info.
        send_buf = msg_without_payload(CODE_GET_SCIUSB)
        
    elif args.cmd == 'injectkey':
        # Read the data in the file.
        data = read_bin_file(args.i)
        if data is None:
            exit(1)
        
        # Inject key.
        send_buf = inject_key(data)
        
    elif args.cmd == 'setboot':
        # Read the data in the file.
        data = read_bin_file(args.hash)
        if data is None:
            exit(1)
        
        # Check whether required options are specified.
        if ROOT_PKEY_HASH_LEN != len(data):
            ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.hash)))
            exit(1)
        
        # Setup secure boot.
        send_buf = set_boot(data, args.enable)
        
    elif args.cmd == 'getboot':
        # Get secure boot info.
        send_buf = msg_without_payload(CODE_GET_BOOT)
        
    elif args.cmd == 'getuid':
        # Get Unique ID.
        send_buf = msg_without_payload(CODE_GET_UID)
        
    elif args.cmd == 'setsauth':
        # Setup SCI/USB boot authentication.
        send_buf = set_sauth(args.mode)
        
    elif args.cmd == 'getsauth':
        # Get SCI/USB boot authentication info.
        send_buf = msg_without_payload(CODE_GET_SCIUSB_AUTH)
        
    elif args.cmd == 'setsauthid':
        # Read the data in the file.
        data = read_bin_file(args.i)
        if data is None:
            exit(1)
        
        # Check whether required options are specified.
        if PASS_INFO_LEN != len(data):
            ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.i)))
            exit(1)
        
        # Setup SCI/USB boot authentication ID.
        send_buf = set_sauthid(data)
        
    elif args.cmd == 'injectcert':
        # Read the data in the file.
        data = read_bin_file(args.i)
        if data is None:
            exit(1)
        
        # Inject certificate.
        send_buf = inject_cert(data)
        
    else: # elif args.cmd == 'start'
        # Read the data in the file.
        data = read_srec_file(args.i)
        if data is None:
            exit(1)
        
        if 'None' != args.ps:
            ps_data = read_bin_file(args.ps)
            if ps_data is None:
                exit(1)
            
            if PASSWORD_LEN != len(ps_data):
                ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.ps)))
                exit(1)
            
        else:
            ps_data = b''
        
        # Transfer the setup program.
        prg_transfer(args.port, data, args.boot_mode, ps_data)
        exit(0)
    
    # Sending and receiving commands to and from the device.
    rcv_buf = setup_by_serial_comm(args.port, send_buf)
    if rcv_buf is None:
        ERRMSG(ERRMSG_RCVCMDFAIL)
        exit(1)
    
    # Check the received response data.
    ret = check_respons_data(rcv_buf, send_buf[1], args.cmd)
    if ret == -1:
        ERRMSG(ERRMSG_RCVCMDFAIL)
        exit(1)
    
    exit(0)

def set_cmdline_argparser():
    """Command line argument parsing settings.
    
    Set valid commands and options on the command line.
    
    Args:
        None
    
    Returns:
        parser - ArgumentParser object
    
    """
    DESC_TOOL       = 'Tools to setup devices'
    DESC_WRITEFLASH = 'Write data to flash'
    DESC_WRITEOTP   = 'Write data to OTP'
    DESC_READOTP    = 'Read data from OTP'
    DESC_SETJAUTH   = 'Set up JTAG authentication'
    DESC_GETJAUTH   = 'Get JTAG authentication information'
    DESC_SETJAUTHID = 'Set the authentication ID to be used as a key during JTAG authentication'
    DESC_SETSCIUSB  = 'Set up SCI/USB boot disabled'
    DESC_GETSCIUSB  = 'Get SCI/USB boot information'
    DESC_INJECTKEY  = 'Inject the key'
    DESC_SETBOOT    = 'Set up secure boot'
    DESC_GETBOOT    = 'Get secure boot information'
    DESC_GETUID     = 'Get Unique ID'
    DESC_SETSAUTH   = 'Set up SCI/USB boot authentication (for RZ/T2H, RZ/T2N, RZ/N2H)'
    DESC_GETSAUTH   = 'Get SCI/USB boot authentication information (for RZ/T2H, RZ/T2N, RZ/N2H)'
    DESC_SETSAUTHID = 'Set the authentication ID to be used as a key during SCI/USB boot authentication (for RZ/T2H, RZ/T2N, RZ/N2H)'
    DESC_INJECTCERT = 'Inject the certificate'
    DESC_START      = 'Transfer the device setup program'
    
    METAVER_ADDR    = '<Address>'
    METAVER_PORT    = '<ComPort>'
    METAVER_BINFILE = '<BinaryFile>'
    METAVER_DATA    = '<Data>'
    METAVER_MODE    = '<Mode>'
    METAVER_SFILE   = '<S-record>'
    
    HELP_CMD        = 'Specify the setup command'
    HELP_WADDR      = 'Specify the write destination address'
    HELP_RADDR      = 'Specify the read destination address'
    HELP_PORT       = 'Specify the COM port number'
    HELP_FLASHI     = 'Specify the path of the data file to write to the flash'
    HELP_DATA       = 'Specify the 16- to 32-bit value to write to the OTP area'
    HELP_JMODE      = 'Specify the JTAG authentication mode (choose from authlv1, authlv2 or prohibit)'
    HELP_JMODE_KEY  = 'Specify the JTAG authentication mode (choose from authlv1 or authlv2)'
    HELP_AUTH_ID    = 'Specify the password info file'
    HELP_HASH       = 'Specify when using hash as the authentication type'
    HELP_KEY_INFO   = 'Specify the key inject info file'
    HELP_BOOT_HASH  = 'Specify root public key hash value for secure boot'
    HELP_BOOT_ENA   = 'Specify when enabling secure boot'
    HELP_LPRG       = 'Specify the S-record file path for the device setup program'
    HELP_BOOT_MODE  = 'Specify the boot mode (choose from sci or usb) '
    HELP_SCIUSB_DIS = 'Specify when disabling SCI/USB boot'
    HELP_SMODE      = 'Specify the SCI/USB boot authentication mode (choose from auth or prohibit)'
    HELP_PS         = 'Specify the path of the auth password file'
    HELP_CERT_INFO  = 'Specify the certificate inject info file'
    
    # Set command line parser.
    parser = argparse.ArgumentParser(description=DESC_TOOL)
    subparsers = parser.add_subparsers(dest='cmd',required=True, help=HELP_CMD)
    parser_writeflash = subparsers.add_parser('writeflash', description=DESC_WRITEFLASH)
    parser_writeotp = subparsers.add_parser('writeotp', description=DESC_WRITEOTP)
    parser_readotp = subparsers.add_parser('readotp', description=DESC_READOTP)
    parser_setjauth = subparsers.add_parser('setjauth', description=DESC_SETJAUTH)
    parser_getjauth = subparsers.add_parser('getjauth', description=DESC_GETJAUTH)
    parser_setjauthid = subparsers.add_parser('setjauthid', description=DESC_SETJAUTHID)
    parser_setsciboot = subparsers.add_parser('setsciboot', description=DESC_SETSCIUSB)
    parser_setusbboot = subparsers.add_parser('setusbboot', description=DESC_SETSCIUSB)
    parser_getsciboot = subparsers.add_parser('getsciboot', description=DESC_GETSCIUSB)
    parser_getusbboot = subparsers.add_parser('getusbboot', description=DESC_GETSCIUSB)
    parser_injectkey = subparsers.add_parser('injectkey', description=DESC_INJECTKEY)
    parser_setboot = subparsers.add_parser('setboot', description=DESC_SETBOOT)
    parser_getboot = subparsers.add_parser('getboot', description=DESC_GETBOOT)
    parser_getuid = subparsers.add_parser('getuid', description=DESC_GETUID)
    parser_setsauth = subparsers.add_parser('setsauth', description=DESC_SETSAUTH)
    parser_getsauth = subparsers.add_parser('getsauth', description=DESC_GETSAUTH)
    parser_setsauthid = subparsers.add_parser('setsauthid', description=DESC_SETSAUTHID)
    parser_injectcert = subparsers.add_parser('injectcert', description=DESC_INJECTCERT)
    parser_start = subparsers.add_parser('start', description=DESC_START)
    
    # Set arguments for writeflash.
    parser_writeflash.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_writeflash.add_argument('--addr', metavar=METAVER_ADDR, required=True, help=HELP_WADDR)
    parser_writeflash.add_argument('-i', metavar=METAVER_BINFILE, required=True, help=HELP_FLASHI)
    
    # Set arguments for writeotp.
    parser_writeotp.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_writeotp.add_argument('--addr', metavar=METAVER_ADDR, required=True, help=HELP_WADDR)
    parser_writeotp.add_argument('--data', metavar=METAVER_DATA, required=True, help=HELP_DATA)
    
    # Set arguments for readotp.
    parser_readotp.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_readotp.add_argument('--addr', metavar=METAVER_ADDR, required=True, help=HELP_RADDR)
    
    # Set arguments for setjauth.
    parser_setjauth.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setjauth.add_argument('--mode', choices=['authlv1', 'authlv2', 'prohibit'], metavar=METAVER_MODE, required=True, help=HELP_JMODE)
    
    # Set arguments for getjauth.
    parser_getjauth.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for setjauthid.
    parser_setjauthid.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setjauthid.add_argument('--mode', choices=['authlv1', 'authlv2'], metavar=METAVER_MODE, required=True, help=HELP_JMODE_KEY)
    parser_setjauthid.add_argument('-i', metavar=METAVER_BINFILE, required=True, help=HELP_AUTH_ID)
    
    # Set arguments for setsciboot.
    parser_setsciboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setsciboot.add_argument('--disable', action='store_true', required=True, help=HELP_SCIUSB_DIS)
    
    # Set arguments for setusbboot.
    parser_setusbboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setusbboot.add_argument('--disable', action='store_true', required=True, help=HELP_SCIUSB_DIS)
    
    # Set arguments for getsciboot.
    parser_getsciboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for getusbboot.
    parser_getusbboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for injectkey.
    parser_injectkey.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_injectkey.add_argument('-i', metavar=METAVER_BINFILE, required=True, help=HELP_KEY_INFO)
    
    # Set arguments for setboot.
    parser_setboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setboot.add_argument('--hash', metavar=METAVER_BINFILE, required=True, help=HELP_BOOT_HASH)
    parser_setboot.add_argument('--enable', action='store_true', help=HELP_BOOT_ENA)
    
    # Set arguments for getboot.
    parser_getboot.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for getuid.
    parser_getuid.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for setsauth.
    parser_setsauth.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setsauth.add_argument('--mode', choices=['auth', 'prohibit'], metavar=METAVER_MODE, required=True, help=HELP_SMODE)
    
    # Set arguments for getsauth.
    parser_getsauth.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    
    # Set arguments for setsauthid.
    parser_setsauthid.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_setsauthid.add_argument('-i', metavar=METAVER_BINFILE, required=True, help=HELP_AUTH_ID)
    
    # Set arguments for injectcert.
    parser_injectcert.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_injectcert.add_argument('-i', metavar=METAVER_BINFILE, required=True, help=HELP_CERT_INFO)
    
    # Set arguments for start.
    parser_start.add_argument('--port', metavar=METAVER_PORT, required=True, help=HELP_PORT)
    parser_start.add_argument('-i', metavar=METAVER_SFILE, required=True, help=HELP_LPRG)
    parser_start.add_argument('--boot_mode', choices=['sci', 'usb'], metavar=METAVER_MODE, required=True, help=HELP_BOOT_MODE)
    parser_start.add_argument('--ps', metavar=METAVER_BINFILE, default='None', help=HELP_PS)
    
    return parser

def write_flash(addr, data):
    """Creation of transmission data (WRITE_FLASH)
    
    Generates flash write command data to send to the device.
    
    Args:
        addr (int): Write address
        data (bytes): Write data
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_WRITE_FLASH
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    payload_size = len(data) + SIZE_WRITE_FLASH
    msg += payload_size.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add flash write destination address.
    msg += addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def write_otp(addr, data):
    """Creation of transmission data (WRITE_OTP)
    
    Generates otp write command data to send to the device.
    
    Args:
        addr (int): Write address
        data (int): Write data
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_WRITE_OTP
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_WRITE_OTP.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add otp write destination address.
    msg += addr.to_bytes(HWORD_BYTES, ENDIAN_LITTLE)
    
    # Add otp write data.
    msg += data.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def read_otp(addr):
    """Creation of transmission data (READ_OTP)
    
    Generates otp read command data to send to the device.
    
    Args:
        addr (int): Read address
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_READ_OTP
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_READ_OTP.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add otp read destination address.
    msg += addr.to_bytes(HWORD_BYTES, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_jauth(mode):
    """Creation of transmission data (SET_JTAG_AUTH)
    
    Generates setup JTAG auth command data to send to the device.
    
    Args:
        mode (str): Authentication mode
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_JTAG_AUTH
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_JTAG_AUTH.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add authentication mode.
    if mode == 'authlv1':
        msg += JTAG_AUTH_MODE_AUTHLV1.to_bytes(1, ENDIAN_LITTLE)
        
    elif mode == 'authlv2':
        msg += JTAG_AUTH_MODE_AUTHLV2.to_bytes(1, ENDIAN_LITTLE)
        
    else:
        msg += JTAG_AUTH_MODE_PROHIBIT.to_bytes(1, ENDIAN_LITTLE)
    
    # Add authentication type. (HASH fixed)
    msg += AUTH_TYPE_HASH.to_bytes(1, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_jauthid(mode, data):
    """Creation of transmission data (SET_JTAG_AUTHID)
    
    Generates setup JTAG authID command data to send to the device.
    
    Args:
        mode (str): Authentication mode
        data (bytes): Password info file
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_JTAG_AUTHID
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_JTAG_AUTHID.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add authentication mode.
    if mode == 'authlv1':
        msg += JTAG_AUTH_MODE_AUTHLV1.to_bytes(1, ENDIAN_LITTLE)
        
    else:
        msg += JTAG_AUTH_MODE_AUTHLV2.to_bytes(1, ENDIAN_LITTLE)
    
    # Add authentication type. (HASH fixed)
    msg += AUTH_TYPE_HASH.to_bytes(1, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_sciusb():
    """Creation of transmission data (SET_SCIUSB)
    
    Generates setup SCI/USB boot disabled command data to send to the device.
    
    Args:
        None
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_SCIUSB
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_SCIUSB.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add SCI/USB boot mode.
    msg += SCI_USB_BOOT_MODE_DISABLE.to_bytes(1, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def inject_key(data):
    """Creation of transmission data (INJECT_KEY)
    
    Generates inject key command data to send to the device.
    
    Args:
        data (bytes): Key inject info file
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_INJECT_KEY
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    payload_size = len(data) + SIZE_INJECT_KEY
    msg += payload_size.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_boot(data, enable_flg):
    """Creation of transmission data (SET_BOOT)
    
    Generates setup secure boot command data to send to the device.
    
    Args:
        data (bytes): RPK hash file
        enable_flg (bool): Secure boot enable flag
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_BOOT
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_BOOT.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add setting flag.
    if True == enable_flg:
        msg += SEC_BOOT_SET_FLAG_ON.to_bytes(1, ENDIAN_LITTLE)
        
    else:
        msg += SEC_BOOT_SET_FLAG_OFF.to_bytes(1, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_sauth(mode):
    """Creation of transmission data (SET_SCIUSB_AUTH)
    
    Generates setup SCI/USB boot auth command data to send to the device.
    (for RZ/T2H, RZ/T2N, RZ/N2H)
    
    Args:
        mode (str): Authentication mode
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_SCIUSB_AUTH
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_SCIUSB_AUTH.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add authentication mode.
    if mode == 'auth':
        msg += SCI_USB_BOOT_AUTH_MODE_AUTH.to_bytes(1, ENDIAN_LITTLE)
        
    else:
        msg += SCI_USB_BOOT_AUTH_MODE_PROHIBIT.to_bytes(1, ENDIAN_LITTLE)
    
    # Add authentication type. (HASH fixed)
    msg += AUTH_TYPE_HASH.to_bytes(1, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def set_sauthid(data):
    """Creation of transmission data (SET_SCIUSB_AUTHID)
    
    Generates setup SCI/USB boot authID command data to send to the device.
    (for RZ/T2H, RZ/T2N, RZ/N2H)
    
    Args:
        data (bytes): Password binary file
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_SET_SCIUSB_AUTHID
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_SET_SCIUSB_AUTHID.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add authentication type. (HASH fixed)
    msg += AUTH_TYPE_HASH.to_bytes(1, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def inject_cert(data):
    """Creation of transmission data (INJECT_CERT)
    
    Generates inject certificate command data to send to the device.
    
    Args:
        data (bytes): Certificate inject info file
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + CODE_INJECT_CERT
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    payload_size = len(data) + SIZE_INJECT_CERT
    msg += payload_size.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add data read from file.
    msg += data
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def msg_without_payload(code):
    """Creation of no payload message
    
    Generates command data that does not include
    the payload to send to the device.
    
    Args:
        code (int): Command code
    
    Returns:
        msg (bytes)
    
    """
    # Add command type and command code.
    msg = TYPE_REQ + code
    
    # Add dummy area.
    msg += DUMMY_AREA_2BYTE_VAL
    
    # Add payload size.
    msg += SIZE_NO_PAYLOAD.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Calculate checksum.
    sum = calc_checksum(msg)
    msg += sum.to_bytes(1, ENDIAN_LITTLE)
    
    return msg

def setup_by_serial_comm(port, send_msg):
    """Setup using serial communication 
    
    Send a request to the device via serial communication.
    After that, it receives a response from the device. 
    
    Args:
        port (str): COM port
        send_msg (bytes): Send command data
    
    Returns:
        rcv_msg (bytes)
    
    """
    try:
        ser = serial.Serial(port, SETUP_BAUDRATE, timeout=SETUP_TIMEOUT)
        
        # Send command.
        ser.write(send_msg)
        
        # Receive command type, command code and dummy area. (4 bytes)
        rcv_msg = ser.read(4)
        if len(rcv_msg) == 0:
            return None
        
        # Receive payload size. (4 bytes)
        size = ser.read(4)
        
        # Receive payload.
        payload_len = int.from_bytes(size, ENDIAN_LITTLE)
        rcv_msg += size
        rcv_msg += ser.read(payload_len)
        
        # Receive checksum. (1 bytes)
        rcv_sum = ser.read()
        
    except Exception:
        pass
        
    finally:
        if 'ser' in locals():
            ser.close()
    
    # Check the length of the received message.
    # Since Response data is at least 9 bytes, it is invalid if it is less than that. 
    # 9 bytes = 1 (Command Type) + 1 (Command Code) + 2 (dummy area) + 4 (Payload Size) + 1 (Return Code) + n
    if len(rcv_msg) < 9:
        return None
        
    else:
        # Verify checksum.
        sum = calc_checksum(rcv_msg)
        
        if sum != rcv_sum[0]:
            return None
    
    return rcv_msg

def check_respons_data(res_data, cmd_code, args_cmd):
    """Check response data
    
    Check the received response data and display the result. 
    
    Args:
        res_data (bytes): Response data
        cmd_code (bytes): Command code
        args_cmd (str): Command specified by argument
    
    Returns:
        Result - "0" if normal, "-1" if abnormal
    
    """
    # Check command type and command code.
    if res_data[0] != TYPE_RES[0] or res_data[1] != cmd_code:
        return -1
    
    # Get return code.
    ret_code = res_data[8]
    
    # Get payload length.
    payload_len = int.from_bytes(res_data[4 : 8], ENDIAN_LITTLE)
    
    # Check the return code and display the result.
    if ret_code == RET_SUCCESS:
        RESMSG(str.format(RESMSG_SUCCESS % (args_cmd)))
        
        # Display the read data.
        if cmd_code == CODE_READ_OTP[0]:
            
            # Read otp area.
            if payload_len == 2:
                read_data = res_data[9 : 11]
            else: # payload_len == 4:
                read_data = res_data[9 : 13]
            
            output_loop = False
            
            for item in read_data:
                if False == output_loop:
                    output_loop = True
                else:
                    print(end=', ')
                
                print(format(item, '#04x'), end='')
            
        elif cmd_code == CODE_GET_JTAG_AUTH[0]:
            # Get JTAG authentication info.
            if JTAG_AUTH_MODE_AUTHLV1 == res_data[9]:
                print('JTAG authentication mode : Authentication lv1')
                
            elif JTAG_AUTH_MODE_AUTHLV2 == res_data[9]:
                print('JTAG authentication mode : Authentication lv2')
                
            elif JTAG_AUTH_MODE_PROHIBIT == res_data[9]:
                print('JTAG authentication mode : Permanent prohibition')
                
            else:
                print('JTAG authentication mode : No authentication')
            
        elif cmd_code == CODE_GET_SCIUSB[0]:
            # Get SCI/USB boot info.
            # Check SCI/USB boot mode.
            if SCI_USB_BOOT_MODE_DISABLE == res_data[9]:
                print('SCI/USB boot : Disable')
                
            else:
                print('SCI/USB boot : Enable')
            
        elif cmd_code == CODE_GET_BOOT[0]:
            # Get secure boot info.
            # Check enable flag.
            if SEC_BOOT_SET_FLAG_ON == res_data[9]:
                print('Secure boot : Enable')
                
            else:
                print('Secure boot : Disable')
            
            read_data = res_data[10 : 42]
            
            print('Hash : ', end='')
            
            for item in read_data:
                print(format(item, '02x'), end='')
            
            print('');
            
        elif cmd_code == CODE_GET_UID[0]:
            # Get Unique ID.
            read_data = res_data[9 : 17]
            
            print('Unique ID : ', end='')
            
            for item in read_data:
                print(format(item, '02x'), end='')
            
            print('');
            
        elif cmd_code == CODE_GET_SCIUSB_AUTH[0]:
            # Get SCI/USB boot authentication info.
            if SCI_USB_BOOT_AUTH_MODE_AUTH == res_data[9]:
                print('SCI/USB boot authentication mode : Authentication')
                
            elif SCI_USB_BOOT_AUTH_MODE_PROHIBIT == res_data[9]:
                print('SCI/USB boot authentication mode : Permanent prohibition')
                
            else:
                print('SCI/USB boot authentication mode : No authentication')
            
    elif ret_code == RET_DATA_FAIL:
        RESMSG(str.format(RESMSG_DATA_FAIL % (args_cmd)))
        
    elif ret_code == RET_WRITE_FAIL:
        RESMSG(str.format(RESMSG_WRITE_FAIL % (args_cmd)))
        
    elif ret_code == RET_READ_FAIL:
        RESMSG(str.format(RESMSG_READ_FAIL % (args_cmd)))
        
    else:
        return -1
    
    return 0

def check_intermediate_respons_data(res_data, cmd_code, args_cmd, write_len, data_len):
    """Check intermediate response data
    
    Check the intermediate received response data.
    Used when dividing data and repeatedly sending Request data.
    
    Args:
        res_data (bytes): Response data
        cmd_code (bytes): Command code
        args_cmd (str): Command specified by argument
        write_len (int): Send data size
        data_len (int): Data size
    
    Returns:
        Result - "0" if normal, "-1" if abnormal 
    
    """
    ret = 0;
    
    # Check command type and command code.
    if res_data[0] != TYPE_RES[0] or res_data[1] != cmd_code:
        ERRMSG(ERRMSG_RCVCMDFAIL)
        ret = -1
    
    else:
        # Get return code.
        ret_code = res_data[8]
        
        # Check the return code and display the result.
        if ret_code == RET_SUCCESS:
            write_len_str  = format(write_len, 'd')
            data_len_str = format(data_len, 'd')
            
            # Continue sending split data.
            RESMSG(str.format(RESMSG_CONT_SEND % (write_len_str, data_len_str)))
        
        elif ret_code == RET_DATA_FAIL:
            RESMSG(str.format(RESMSG_DATA_FAIL % (args_cmd)))
            ret = -1
        
        elif ret_code == RET_WRITE_FAIL:
            RESMSG(str.format(RESMSG_WRITE_FAIL % (args_cmd)))
            ret = -1
        
        else:
            ERRMSG(ERRMSG_RCVCMDFAIL)
            ret = -1
    
    return ret

def prg_transfer(port, prg_data, boot_mode, ps_data):
    """Check response data
    
    Check the received response data and display the result. 
    
    Args:
        port (str): COM port
        prg_data (bytes): Program data
        boot_mode (str): SCI/USB boot mode
        ps_data (bytes): Password data
    
    Returns:
        None
    
    """
    try:
        # Check SCI/USB boot type.
        if PASSWORD_LEN == len(ps_data):
            boot_type = BOOT_AUTH
        else:
            boot_type = BOOT_NORMAL
        
        # Set up serial communication.
        ser = serial.Serial(port, BOOT_BAUDRATE, timeout=BOOT_TIMEOUT)
        
        if 'usb' == boot_mode:
            ser.write(S0_OPEN.encode())
            
            # Log output during transfer.
            if 0 == START_CMD_LOG_DISABLE:
                print('USB Open.')
            
            # Receive message.
            line = ser.readline()
            rcv_str = line.decode()
            
            # Log output during transfer.
            if 0 == START_CMD_LOG_DISABLE:
                print(rcv_str, end="")
            
        else:
            # Log output during transfer.
            if 0 == START_CMD_LOG_DISABLE:
                print('SCI Download mode.')
        
        # Generate S0 HDR.
        s0_data = s0_hdr_msg[boot_type].encode()
        
        # Generate S0 Password data.
        if BOOT_AUTH == boot_type:
            data = b'\x13\x00\x00' + ps_data
            sum = calc_checksum(data)
            sum = BYTE_MAX_NUM - sum;
            data += sum.to_bytes(1, ENDIAN_LITTLE)
            s0_ps_msg = 'S0'
            
            for item in data:
                s0_ps_msg += format(item, '02X')
                
            s0_ps_msg += '\r\n'
            s0_data += s0_ps_msg.encode()
        
        # Send S0 message.
        ser.write(s0_data)
        
        # Log output during transfer.
        if 0 == START_CMD_LOG_DISABLE:
            print('Send program data. (S0)')
        
        # Receive message.
        line = ser.readline()
        rcv_str = line.decode()
        
        # Log output during transfer.
        if 0 == START_CMD_LOG_DISABLE:
            print(rcv_str, end="")
        
        # S3 format S-Record.
        ser.write(prg_data)
        
        # Log output during transfer.
        if 0 == START_CMD_LOG_DISABLE:
            print('Send program data. (S3)')
            
            # Receive message.
            # The transfer itself is complete, so if you disable output,
            # the last message will not be received.
            line = ser.readline()
            rcv_str = line.decode()
            print(rcv_str, end="")
        
        else:
            # When log output is disabled, the serial port may be closed after sending S3 without waiting for the sequence on the device to end, 
            # so it will wait until the sequence ends.
            # When log output is enabled, the serial port will be closed after receiving the final message from the device, 
            # so there is no need to wait.
            time.sleep(1)
        
    except Exception:
        print(RETMSG_START_FAIL)
        pass
        
    finally:
        if 'ser' in locals():
             ser.close()
    
    return

def calc_checksum(msg):
    """Calculate checksum 
    
    Calculate the checksum of the message.
    
    Args:
        msg (bytes): Message
    
    Returns:
        sum (int)
    
    """
    sum = 0
    for item in msg:
        sum += item
    sum %= (BYTE_MAX_NUM + 1)
    
    return sum

def read_bin_file(file_path):
    """Read binary data from file.
    
    Reads data from the file in binary format. 
    
    Args:
        file_path (str) - Binary file path
    
    Returns:
        bin (bytes)
    
    """
    try:
        with open(file_path, 'rb') as infile:
            bin = infile.read()
    except:
        ERRMSG(str.format(ERRMSG_OPEN_FAIL % (file_path)))
        bin = None
    return bin

def read_srec_file(file_path):
    """Read S record data file.
    
    Read data from the file except for S0 and line breaks after S7.
    
    Args:
        file_path (str) - S record file path
    
    Returns:
        data (bytes)
    
    """
    try:
        # Read data. (str)
        with open(file_path, 'r', newline='\r\n') as infile:
            
            # Check S0.
            s0_line = infile.readline()
            
            # If S0 does not exist, move to the top of the file.
            if True != s0_line.startswith('S0'):
                infile.seek(0)
            
            # Read data from the file except for S0.
            data_file = infile.read()
            
            # Except for unnecessary line break after S7.
            data = data_file.rstrip("\r\n")
            
            # Encode strings.
            encode_data = data.encode()
            
    except:
        ERRMSG(str.format(ERRMSG_OPEN_FAIL % (file_path)))
        encode_data = None
    return encode_data

if __name__=='__main__':
    main()

