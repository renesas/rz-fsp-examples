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
# fwupdate_utility.py : Tool Version 3.00-t2n-beta
import binascii
import os
import re
import sys
import argparse

# External constant (can be changed by the user at will)
PADDING_VAL  = 'FF'     # Padding value in setupfile
PRODUCT_NAME = 'sample' # Product name to be set in updatefile (specify within 32 characters)

# Error message print
ERRMSG = print
WARMSG = print

# Endian string
ENDIAN_BIG    = 'big'
ENDIAN_LITTLE = 'little'

# Error/Warning Message
ERRMSG_FILE_SIZE_FAIL = __file__ + ' : error: The file size is invalid : %s'
ERRMSG_OPEN_FAIL      = __file__ + ' : error: No such file : %s'
ERRMSG_ADDR_FAIL      = __file__ + ' : error: The input value of the address is invalid : %s'
ERRMSG_TLV_FAIL       = __file__ + ' : error: The format of product name is invalid : %s'
WARMSG_PARAM_EMPTY    = __file__ + ' : warning: The file name of the "parameter information" is omitted'
WARMSG_UNUSED_PARAM   = __file__ + ' : warning: The %s was discarded because it was not used'

# Update Info header param
MAGIC_NUMBER             = 0x75706469
UPDATE_TARGET_CPU_CORE_0 = 0x0
UPDATE_TARGET_CPU_CORE_1 = 0x1
UPDATE_TARGET_CPU_CORE_2 = 0x2
UPDATE_TARGET_CPU_CORE_3 = 0x3
PARAM_INFO_POS           = (4)
PARAM_INFO_AVAILABLE     = (0x1 << (PARAM_INFO_POS))
PARAM_INFO_NOT_AVAILABLE = (0x0 << (PARAM_INFO_POS))

# Type Class
TLV_TYPE_CLS_POS      = (28)  # Type Class Bit Position
TLV_TYPE_CLS_FWUPDATE = (0x6 << (TLV_TYPE_CLS_POS))   # Class FWUPDATE

# Use type of FWUpdate class
TLV_TYPE_CLS_FWUP_UT_POS     = (24)  # Use Type Bit Position
TLV_TYPE_CLS_FWUP_UT_PRODUCT = (0x0 << (TLV_TYPE_CLS_FWUP_UT_POS))

# TLV mask
TLV_TYPE_MASK = 0xFFFFFF00
TLV_LEN_MASK  = 0x000000FF

# TLV length
TLV_VAL_LEN_MIN = 0
TLV_VAL_LEN_MAX = 1020

# Value for setting
BOOT_STATE_A                 = 0x10011001
BOOT_STATE_B                 = 0x01100110
FLASH_MNG_AREA_CPU_CORE0_VAL = BOOT_STATE_A
FLASH_MNG_AREA_CPU_CORE1_VAL = BOOT_STATE_A
FLASH_MNG_AREA_CPU_CORE2_VAL = BOOT_STATE_A
FLASH_MNG_AREA_CPU_CORE3_VAL = BOOT_STATE_A
RESERVED_VAL                 = '00'

###################################################################################################################################
# Supplementary information about [SSBL_AREA_MAX_SIZE_RZ*] and [UPDATE_PROG_AREA_MAX_SIZE_RZ*]                                    #
# The above two constants are used when creating a Setup file.                                                                    #
#                                                                                                                                 #
# The memory map for each device is shown below.                                                                                  #
# In the memory map shown below, [SSBL_AREA_MAX_SIZE_RZ*] corresponds to the maximum size of the [SSBL area].                     #
# [UPDATE_PROG_AREA_MAX_SIZE_RZ*] corresponds to the maximum size of the [Update program area].                                   #
# Note: The memory map shown below is for RZ/T2M xSPI0 flash.                                                                     #
#       The size of each area and the memory location are the same for other flashes.                                             #
#       For CA55 devices the memory map may be different.                                                                         #
#                                                                                                                                 #
# (Memory map)                                                                                                                    #
# Address Range           | Size                            | Description                                                         #
# 0x60000000 - 0x6000004F | LOADER_PARAM_BYTE_SIZE + 4 Byte | Area for the parameter for the loader                               #
# 0x60000050 - 0x6000704F | SSBL_AREA_MAX_SIZE_RZ*          | SSBL area                                                           #
# 0x60007050 - 0x6002FFFF | UPDATE_PROG_AREA_MAX_SIZE_RZ*   | Update program area                                                 #
# 0x60030000 - 0x6003000F | PARAM_BYTE_SIZE                 | Area for the parameter for the user application program (CPU Core0) #
# 0x60030010 - 0x6003001F | PARAM_BYTE_SIZE                 | Area for the parameter for the user application program (CPU Core1) #
# 0x60030020 - 0x6003002F | 16 Byte                         | Plane management area                                               #
# 0x60030030 - 0x63FFFFFF | 63 MB                           | Area for User application program                                   #
###################################################################################################################################

# SSBL area max size
SSBL_AREA_MAX_SIZE_RZT2H = 0x37000
SSBL_AREA_MAX_SIZE_RZT2N = 0x37000
SSBL_AREA_MAX_SIZE_RZN2H = 0x37000
SSBL_AREA_MAX_SIZE_RZT2M = 0x7000
SSBL_AREA_MAX_SIZE_RZT2L = 0x7000
SSBL_AREA_MAX_SIZE_RZN2L = 0x7000

# SSBL area max size list
ssbl_area_max_size = {
    'rzt2h':   SSBL_AREA_MAX_SIZE_RZT2H,
    'rzt2n':   SSBL_AREA_MAX_SIZE_RZT2N,
    'rzn2h':   SSBL_AREA_MAX_SIZE_RZN2H,
    'rzt2m':   SSBL_AREA_MAX_SIZE_RZT2M,
    'rzt2l':   SSBL_AREA_MAX_SIZE_RZT2L,
    'rzn2l':   SSBL_AREA_MAX_SIZE_RZN2L,
}

# Update program area max size
UPDATE_PROG_AREA_MAX_SIZE_RZT2H = 0x5CFB0
UPDATE_PROG_AREA_MAX_SIZE_RZT2N = 0x5CFB0
UPDATE_PROG_AREA_MAX_SIZE_RZN2H = 0x5CFB0
UPDATE_PROG_AREA_MAX_SIZE_RZT2M = 0x28FB0
UPDATE_PROG_AREA_MAX_SIZE_RZT2L = 0x28FB0
UPDATE_PROG_AREA_MAX_SIZE_RZN2L = 0x28FB0

# Update program area max size list
update_prog_area_max_size = {
    'rzt2h':   UPDATE_PROG_AREA_MAX_SIZE_RZT2H,
    'rzt2n':   UPDATE_PROG_AREA_MAX_SIZE_RZT2N,
    'rzn2h':   UPDATE_PROG_AREA_MAX_SIZE_RZN2H,
    'rzt2m':   UPDATE_PROG_AREA_MAX_SIZE_RZT2M,
    'rzt2l':   UPDATE_PROG_AREA_MAX_SIZE_RZT2L,
    'rzn2l':   UPDATE_PROG_AREA_MAX_SIZE_RZN2L,
}

# Size
LOADER_PARAM_BYTE_SIZE = 76 # Data size of Parameter for the loader.
PARAM_BYTE_SIZE        = 16 # Data size of Parameter for the User application.
PRODUCT_LEN_MAX        = 32
WORD_BYTES             = 4

def main():
    """Main processing.
    
    Parsing command line arguments.
    Create firmware update file.
    
    Args:
        None
        
    Returns:
        None
        
    """
    # Command line argument perse.
    parser = set_cmdline_argparser()
    args = parser.parse_args()
    
    # Update file generation command.
    # First, check the arguments for errors.
    if 'updatefile' == args.cmd:
    
        # Perform error check for write_addr.
        if re.match("^[0-9A-Fa-f]{8}$", args.write_addr) is None:
            ERRMSG(str.format(ERRMSG_ADDR_FAIL % (args.write_addr)))
            exit(1)
            
        # Write Address setting.
        write_addr = int(args.write_addr, 16)
        
        # Check the existence of Firmware data.
        fw = read_bin_file(args.i)
        if fw is None:
            exit(1)
            
        if args.param is not None:
            # Check the existence of Userapp Parameter.
            param = read_bin_file(args.param)
            if param is None:
                exit(1)
                
            # Userapp Parameter length check.
            if PARAM_BYTE_SIZE != len(param):
                ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param)))
                exit(1)
                
        else: # args.param is None
            param = None
            
        # Convert cpu core number from str to hexadecimal
        cpu_core_num = int(args.cpu_core, 16)
        
        # Call a function that generates update file data.
        outbuf = make_updatefile(write_addr, fw, cpu_core_num, param)
        
    # Setup file generation command.
    # First, check the arguments for errors.
    elif 'setupfile' == args.cmd:
        # Check the existence of Loader Parameter.
        loader_buf = read_bin_file(args.param_loader)
        if loader_buf is None:
            exit(1)
            
        # Loader Parameter length check.
        if LOADER_PARAM_BYTE_SIZE > len(loader_buf):
            ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param_loader)))
            exit(1)
            
        # Check if ssbl option is specified.
        if args.ssbl is not None:
            ssbl_buf = read_bin_file(args.ssbl)
            if ssbl_buf is None:
                exit(1)
        else:
            # Initialize ssbl_buf.
            ssbl_buf = bytearray()
            
        # Check the existence of Userapp Parameter(CPU Core0).
        param_cpu_core_0 = read_bin_file(args.param_cpu_core_0)
        if param_cpu_core_0 is None:
            exit(1)
            
        # Userapp Parameter(CPU Core0) length check.
        if PARAM_BYTE_SIZE != len(param_cpu_core_0):
            ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param_cpu_core_0)))
            exit(1)
            
        # Check that option param_cpu_core_1 is specified.
        if args.param_cpu_core_1 is not None:
            # Check the existence of Userapp Parameter(CPU Core1).
            param_cpu_core_1 = read_bin_file(args.param_cpu_core_1)
            if param_cpu_core_1 is None:
                exit(1)
                
            # Userapp Parameter(CPU Core1) length check.
            if PARAM_BYTE_SIZE != len(param_cpu_core_1):
                ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param_cpu_core_1)))
                exit(1)
                
        else: # elif args.param_cpu_core_1 is None
            # Generate 16 bytes data to be set to Userapp Parameter(CPU Core1) Area.
            param_cpu_core_1 = bytes.fromhex(PADDING_VAL * PARAM_BYTE_SIZE)
            
        # Check that option param_cpu_core_2 is specified.
        if args.param_cpu_core_2 is not None:
            # Check the existence of Userapp Parameter(CPU Core2).
            param_cpu_core_2 = read_bin_file(args.param_cpu_core_2)
            if param_cpu_core_2 is None:
                exit(1)
                
            # Userapp Parameter(CPU Core2) length check.
            if PARAM_BYTE_SIZE != len(param_cpu_core_2):
                ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param_cpu_core_2)))
                exit(1)
                
            # Check for unused parameter information.
            if ('rzt2m' == args.mpu) or ('rzt2l' == args.mpu) or ('rzn2l' == args.mpu):
                # A warning message will be output for unused parameter information.
                WARMSG(str.format(WARMSG_UNUSED_PARAM % (args.param_cpu_core_2)))
                
        else: # elif args.param_cpu_core_2 is None
            # Generate 16 bytes data to be set to Userapp Parameter(CPU Core2) Area.
            param_cpu_core_2 = bytes.fromhex(PADDING_VAL * PARAM_BYTE_SIZE)
            
        # Check that option param_cpu_core_3 is specified.
        if args.param_cpu_core_3 is not None:
            # Check the existence of Userapp Parameter(CPU Core3).
            param_cpu_core_3 = read_bin_file(args.param_cpu_core_3)
            if param_cpu_core_3 is None:
                exit(1)
                
            # Userapp Parameter(CPU Core3) length check.
            if PARAM_BYTE_SIZE != len(param_cpu_core_3):
                ERRMSG(str.format(ERRMSG_FILE_SIZE_FAIL % (args.param_cpu_core_3)))
                exit(1)
                
            # Check for unused parameter information.
            if ('rzt2m' == args.mpu) or ('rzt2l' == args.mpu) or ('rzn2l' == args.mpu):
                # A warning message will be output for unused parameter information.
                WARMSG(str.format(WARMSG_UNUSED_PARAM % (args.param_cpu_core_3)))
                
        else: # elif args.param_cpu_core_3 is None
            # Generate 16 bytes data to be set to Userapp Parameter(CPU Core3) Area.
            param_cpu_core_3 = bytes.fromhex(PADDING_VAL * PARAM_BYTE_SIZE)
            
        # Check the existence of Update Program.
        update_prog = read_bin_file(args.update_prog)
        if update_prog is None:
            exit(1)
            
        # Generate setupfile data.
        outbuf = make_setupfile(loader_buf, ssbl_buf, param_cpu_core_0, param_cpu_core_1, param_cpu_core_2, param_cpu_core_3, update_prog, args.mpu)
        
    if outbuf is None:
        exit(1)
        
    # Output the file specified by the command.
    write_result = write_bin_file(args.o, outbuf)
    if write_result is None:
        exit(1)
        
    exit(0)

def make_updatefile(write_addr, fw, cpu_core_num, param):
    """Make updatefile.
    
    Args:
        write_addr   (int)   - Firmware write destination address
        fw           (bytes) - Firmware to update
        cpu_core_num (int)   - CPU core number to be updated
        param        (bytes) - Parameter for Userapp after update
        
    Returns:
        Update file (bytes)
        
    """
    if param is not None:
        # If Userapp Parameter is specified on the command line.
        param_bit_field = PARAM_INFO_AVAILABLE
        
        # Set the total size of Userapp and Userapp Parameter.
        img_size = len(fw) + len(param)
        
    else:
        # If no Userapp Parameter is specified on the command line.
        param_bit_field = PARAM_INFO_NOT_AVAILABLE
        
        # Set only Firmware size.
        img_size = len(fw)
        
        # Display a message that Userapp Parameter is not specified.
        WARMSG(WARMSG_PARAM_EMPTY)
        
    # Determine the number of the cpu core to be updated.
    if UPDATE_TARGET_CPU_CORE_0 == cpu_core_num:
        cpu_core_bit_field = UPDATE_TARGET_CPU_CORE_0
        
    elif UPDATE_TARGET_CPU_CORE_1 == cpu_core_num:
        cpu_core_bit_field = UPDATE_TARGET_CPU_CORE_1
        
    elif UPDATE_TARGET_CPU_CORE_2 == cpu_core_num:
        cpu_core_bit_field = UPDATE_TARGET_CPU_CORE_2
        
    else: # UPDATE_TARGET_CPU_CORE_3 == cpu_core_num
        cpu_core_bit_field = UPDATE_TARGET_CPU_CORE_3
        
    # Set bit field.
    update_target = param_bit_field | cpu_core_bit_field
    
    #Process to create Update Info header.
    outbuf = make_update_info_header(write_addr, img_size, update_target)
    if outbuf is None:
        ERRMSG(str.format(ERRMSG_TLV_FAIL % ('Fwupdate')))
        return None
        
    # Add Userapp Parameter.
    if param is not None:
        outbuf += param
        
    # Add Firmware.
    outbuf += fw
    
    # Add CRC.
    outbuf += binascii.crc32(outbuf).to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    return outbuf

def make_setupfile(loader_buf, ssbl_buf, param_cpu_core_0, param_cpu_core_1, param_cpu_core_2, param_cpu_core_3, update_prog, mpu):
    """Make setupfile.
    
    Args:
        loader_buf       (bytes) - Parameter for laoder
        ssbl_buf         (bytes) - Buffer to store SSBL data
        param_cpu_core_0 (bytes) - Parameter for Userapp(CPU Core0)
        param_cpu_core_1 (bytes) - Parameter for Userapp(CPU Core1)
        param_cpu_core_2 (bytes) - Parameter for Userapp(CPU Core2)
        param_cpu_core_3 (bytes) - Parameter for Userapp(CPU Core3)
        update_prog      (bytes) - Update program
        mpu              (str)   - MPU to be updated
        
    Returns:
        Setup file (bytes)
        
    """
    # Get loader param (76 bytes).
    loader_param = loader_buf[:LOADER_PARAM_BYTE_SIZE]
    
    # Padding the last 4 bytes of loader param.
    loader_param += bytes.fromhex(PADDING_VAL * 4)
    
    # Store loader param data in buffer.
    outbuf = loader_param
    
    # Check if SSBL is bound to loader param.
    if ((LOADER_PARAM_BYTE_SIZE + 4) < len(loader_buf)) and (len(ssbl_buf) == 0):
        ssbl_buf = loader_buf[LOADER_PARAM_BYTE_SIZE + 4:]
        
    # Padding of Loader Parameter and SSBL area.
    if len(ssbl_buf) < ssbl_area_max_size[mpu]:
        ssbl_buf += bytes.fromhex(PADDING_VAL * (ssbl_area_max_size[mpu] - len(ssbl_buf)))
        
    # Add SSBL data.
    outbuf += ssbl_buf
    
    # Padding of Update program area.
    if len(update_prog) < update_prog_area_max_size[mpu]:
        update_prog += bytes.fromhex(PADDING_VAL * (update_prog_area_max_size[mpu] - len(update_prog)))
        
    # Add Update program data.
    outbuf += update_prog
    
    # Add Userapp Parameter(CPU Core0).
    outbuf += param_cpu_core_0
    
    # Add Userapp Parameter(CPU Core1).
    outbuf += param_cpu_core_1
    
    if ('rzt2h' == mpu) or ('rzt2n' == mpu) or ('rzn2h' == mpu):
        # Add Userapp Parameter(CPU Core2).
        outbuf += param_cpu_core_2
        
        # Add Userapp Parameter(CPU Core3).
        outbuf += param_cpu_core_3
        
    else: # ('rzt2m' == mpu) or ('rzt2l' == mpu) or ('rzn2l' == mpu)
        # It does nothing for other devices.
        pass
        
    # Add the value for flash mng area(CPU Core0).
    outbuf += FLASH_MNG_AREA_CPU_CORE0_VAL.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add the value for flash mng area(CPU Core1).
    outbuf += FLASH_MNG_AREA_CPU_CORE1_VAL.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    if ('rzt2h' == mpu) or ('rzn2h' == mpu):
        # Add the value for flash mng area(CPU Core2).
        outbuf += FLASH_MNG_AREA_CPU_CORE2_VAL.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        
        # Add the value for flash mng area(CPU Core3).
        outbuf += FLASH_MNG_AREA_CPU_CORE3_VAL.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        
    else: # ('rzt2n' == mpu) or ('rzt2m' == mpu) or ('rzt2l' == mpu) or ('rzn2l' == mpu):
        # Add 0 in reserved area of flash mng area
        outbuf += bytes.fromhex(RESERVED_VAL * (WORD_BYTES * 2))
        
    return outbuf

def make_update_info_header(write_addr, img_size, update_target):
    """Make Update Info header.
    
    Args:
        write_addr    (int) - Firmware write destination address
        img_size      (int) - Total size of userapp and parameter information
        update_target (int) - Bitfield of information required for update
                              (0bit):CPU to be updated
                              (1bit):[Available] or [Not available] parameter information
                              
    Returns:
        Update Info header (bytes)
        
    """
    # Add Magic Number.
    outbuf = MAGIC_NUMBER.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add zero padding to Reserved space1(8 bytes).
    outbuf += bytes.fromhex(RESERVED_VAL * (WORD_BYTES*2))
    
    # Add the Write address.
    outbuf += write_addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add zero padding to Reserved space2.
    outbuf += bytes.fromhex(RESERVED_VAL * (WORD_BYTES))
    
    # Add the Image Size.
    outbuf += img_size.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add zero padding to Reserved space3.
    outbuf += bytes.fromhex(RESERVED_VAL * (WORD_BYTES))
    
    # Add the Update Target
    outbuf += update_target.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Process to create TLV.
    tlv_type = TLV_TYPE_CLS_FWUPDATE | TLV_TYPE_CLS_FWUP_UT_PRODUCT
    
    # Encode the product name.
    # Zero padding for areas less than 32 bytes.
    tlv_value = (PRODUCT_NAME.encode() + bytes.fromhex(RESERVED_VAL * (PRODUCT_LEN_MAX - len(PRODUCT_NAME))))
    
    tlv = make_tlv(tlv_type, tlv_value, ENDIAN_LITTLE, ENDIAN_BIG)
    if tlv is None:
        return None
        
    #Add the TLV length.
    tlv_len = len(tlv)
    outbuf += tlv_len.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    outbuf += tlv
    
    return outbuf

def make_tlv(tlv_type, tlv_value, tl_endian, v_endian):
    """Make TLV.
    
    Creates data for the TLV field according to the specified value.
    
    Args:
        tlv_type  (int)   - TLV type
        tlv_value (bytes) - TLV value data
        tl_endian (str)   - Type length endian
        v_endian  (str)   - Value endian
        
    Returns:
        TLV field (bytes)
        
    """
    # Check value type
    if type(tlv_value) == str:
        try:
            # Covert string to bytes
            tlv_value = bytes.fromhex(tlv_value)
        except:
            return None
            
    # TLV length
    tlv_val_len = len(tlv_value)
    
    # Check multiples of length
    if (tlv_val_len % WORD_BYTES) != 0:
        # length supports only word size multiples
        return None
        
    # Check minimum and maximum length
    if (tlv_val_len < TLV_VAL_LEN_MIN) or (tlv_val_len > TLV_VAL_LEN_MAX):
        return None
        
    # Make TLV
    type_len = (tlv_type & TLV_TYPE_MASK) | ((tlv_val_len//WORD_BYTES) & TLV_LEN_MASK)
    tlv = type_len.to_bytes(WORD_BYTES, tl_endian)
    for i in range(0, tlv_val_len, WORD_BYTES):
        wordval = int.from_bytes(tlv_value[i:i+WORD_BYTES], ENDIAN_BIG)
        tlv += wordval.to_bytes(WORD_BYTES, v_endian)
        
    return tlv

def read_bin_file(file_path):
    """Read binary data from file.
    
    Reads data from the file in binary format.
    
    Args:
        file_path (str) - Binary file path
        
    Returns:
        Bytes read from file (bytes)
        
    """
    try:
        with open(file_path, 'br') as infile:
            bin = infile.read()
    except:
        ERRMSG(str.format(ERRMSG_OPEN_FAIL % (file_path)))
        bin = None
    return bin

def write_bin_file(write_file, outbuf):
    """Write binary data from file.
    
    Write data to the file in binary format. 
    
    Args:
        file_path (str) - Binary file path
        
    Returns:
        Number of characters written to the file (int)
        
    """
    try:
        with open(write_file, 'bw+') as outfile:
            result = outfile.write(outbuf)
    except:
        ERRMSG(str.format(ERRMSG_OPEN_FAIL % (write_file)))
        result = None
    return result

def set_cmdline_argparser():
    """Command line argument parsing settings.
    
    Set valid commands and options on the command line.
    
    Args:
        None
        
    Returns:
        parser - ArgumentParser object
        
    """
    DESC_TOOL       = 'Tools to create update-related files'
    DESC_UPDATEFILE = 'Create update files'
    DESC_SETUPFILE  = 'Create setup files'
    
    # Local constant value
    CPU_CORE_NUM = ['0', '1', '2', '3']
    MPU_TYPE     = ['rzt2h', 'rzt2n', 'rzn2h', 'rzt2m', 'rzt2l', 'rzn2l']
    
    # Command option help message.
    METAVER_HEX      = '<HexConstant>'
    METAVER_FILE     = '<File>'
    METAVER_CPU_CORE = '<0 to 3>'
    METAVER_MPU      = '<MPU Type>'
    
    MSG_HELP_CMD         = 'Specify the type of file to create with the command'
    MSG_HELP_CPU_CORE    = 'Specify the update core number.'
    MSG_HELP_MPU_TYPE    = 'Specify the MPU to be updated.'
    MSG_HELP_LOADER      = 'Specify the file name of the loader parameter file to be input to ' + __file__
    MSG_HELP_SSBL        = 'Specify the file name of the SSBL file to be input to ' + __file__
    MSG_HELP_PARAM       = 'Specify the file name of the userapp parameter infomation file to be input to ' + __file__
    MSG_HELP_UPDATE_PROG = 'Specify the file name of the update program file to be input to ' + __file__
    MSG_HELP_WRITE_ADDR  = 'Specify the write address to which the Userapp to be used for the update'
    MSG_HELP_I           = 'Specify the file name of the Userapp file to be input to ' + __file__
    MSG_HELP_O           = 'Specify the file name of the update file output by ' + __file__
    
    # Set command line parser.
    parser = argparse.ArgumentParser(description=DESC_TOOL)
    subparsers = parser.add_subparsers(dest='cmd',required=True, help=MSG_HELP_CMD)
    parser_updatefile = subparsers.add_parser('updatefile', description=DESC_UPDATEFILE)
    parser_setupfile = subparsers.add_parser('setupfile', description=DESC_SETUPFILE)
    
    # Set arguments for updatefile.
    # Required command
    parser_updatefile.add_argument('--write_addr', help=MSG_HELP_WRITE_ADDR, metavar=METAVER_HEX, required=True)
    parser_updatefile.add_argument('-i', help=MSG_HELP_I, metavar=METAVER_FILE, required=True)
    parser_updatefile.add_argument('-o', help=MSG_HELP_O, metavar=METAVER_FILE, required=True)
    
    # Optional command
    parser_updatefile.add_argument('--param', help=MSG_HELP_PARAM, metavar=METAVER_FILE)
    parser_updatefile.add_argument('--cpu_core', choices=CPU_CORE_NUM, help=MSG_HELP_CPU_CORE, metavar=METAVER_CPU_CORE, default='0')
    
    # Set arguments for setupfile.
    # Required command
    parser_setupfile.add_argument('--param_loader', help=MSG_HELP_LOADER, metavar=METAVER_FILE, required=True)
    parser_setupfile.add_argument('--update_prog', help=MSG_HELP_UPDATE_PROG, metavar=METAVER_FILE, required=True)
    parser_setupfile.add_argument('--param_cpu_core_0', help=MSG_HELP_PARAM, metavar=METAVER_FILE, required=True)
    parser_setupfile.add_argument('--mpu', choices=MPU_TYPE, metavar=METAVER_MPU, required=True, help=MSG_HELP_MPU_TYPE)
    parser_setupfile.add_argument('-o', help=MSG_HELP_O, metavar=METAVER_FILE, required=True)
    
    # Optional command
    parser_setupfile.add_argument('--ssbl', help=MSG_HELP_SSBL, metavar=METAVER_FILE)
    parser_setupfile.add_argument('--param_cpu_core_1', help=MSG_HELP_PARAM, metavar=METAVER_FILE)
    parser_setupfile.add_argument('--param_cpu_core_2', help=MSG_HELP_PARAM, metavar=METAVER_FILE)
    parser_setupfile.add_argument('--param_cpu_core_3', help=MSG_HELP_PARAM, metavar=METAVER_FILE)
    
    return parser

if __name__=='__main__':
    main()
