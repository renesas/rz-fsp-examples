# /***********************************************************************************************************************
# * Copyright (c) 2026 Renesas Electronics Corporation and/or its affiliates
# *
# * SPDX-License-Identifier: BSD-3-Clause
# ***********************************************************************************************************************/

# parameter_generator.py : Tool Version 420
import os
import re
import argparse

# Message print
ERRMSG = print
LOGMSG = print

# Endian string
ENDIAN_BIG            = 'big'
ENDIAN_LITTLE         = 'little'

# Size definition for calculation.
WORD_BYTES            = 4
WORD_BYTES_MAX_NUM    = 4294967295

# Error / Warning / Notice Message
ERRMSG_OPENFAIL = __file__ + ' : error: No such file : %s'
ERRMSG_ADDRFAIL = __file__ + ' : error: The input value of the address is invalid : %s'
ERRMSG_SIZEFAIL = __file__ + ' : error: The file size limit has been exceeded : %s'
ERRMSG_MODEFAIL = __file__ + ' : error: The mode specified by MPU is not supported : %s'
ERRMSG_CPUFAIL  = __file__ + ' : error: The CPU specified by MPU is not supported : %s'
ERRMSG_UNITSIZE = __file__ + ' : error: The file size is not a multiple of the unit size : %s'
NOTICE_DESTADDR = __file__ + ' : notice: For secure boot, the start address of the loader program must be the specified address plus 0x800 : %s'
NOTICE_LIMIT_SIZE_T2N_CR52_BTCM = __file__ + ' : notice: It has been exceeded the size that can be deployed to BTCM. If you want to deploy RZ/T2N CR52 on BTCM, be sure to keep it within 52KB. Current Size : %s'

# Fixed value included in parameter information for loader.
# Refer to UM's Parameter information for the loader and change the values as needed.
# Settings common to each device.
CACHE_FLG             = 0x00000001       # Cache enable flag during boot process : Enable
INIT_FLG              = 0x22360679       # Flag to initialize xSPI/Bus settings after boot process : Enable
CS0BCR                = 0x36DB0C00       # CS0 bus control register (CS0BCR) initial value
CS0WCR                = 0x00000500       # CS0 wait control register (CS0BCR) initial value
WRAPCFG               = 0x00000000       # Wrapper Configuration register (WRAPCFG) initial value
COMCFG                = 0x00000000       # Common Configuration register (COMCFG) initial value
BMCFG                 = 0x00000000       # Bridge Map Configuration register (BMCFG) initial value
LIOCFGCS0             = 0x00070000       # link I/O Configuration Register CS0 (LIOCFGCS0) initial value
ACCESS_SPEED          = 0x00000606       # External memory access speed setting value
# for RZ/T2M, RZ/T2L and RZ/N2L.
CSSCTL                = 0x00000000       # CS Size Control register (CSSCTL) initial value
# for RZ/T2H, RZ/T2N, RZ/N2H.
PLL0_SSC_CTR          = 0x00000000       # PLL0 SSC Control Register (PLL0_SSC_CTR) initial value

# Definition for padding
PADDING_VAL           = 'FF'             # Value at the time of padding

# Size definition related to parameter information.
SEC_IMAGE_UNIT_SIZE          = 16          # The size unit of the secure boot image
LOADER_CERT_LEN              = 0x0800      # Certificate size for Loader
USERAPP_CERT_LEN             = 0x0200      # Certificate size for UserApp
LOADER_SIZE_MAX_RZT2H_BTCM   = 0x0000D000  # Maximum size of loader program: 52 KByte (for RZ/T2H, RZ/N2H BTCM)
LOADER_SIZE_MAX_RZT2H_SYSRAM = 0x001FD000  # Maximum size of loader program: 2036 KByte (for RZ/T2H, RZ/N2H SystemRAM)
LOADER_SIZE_MAX_RZT2N_BTCM   = 0x0000D000  # Maximum size of loader program: 52 KByte (for RZ/T2N BTCM)
LOADER_SIZE_MAX_RZT2N_SYSRAM = 0x001FD000  # Maximum size of loader program: 2036 KByte (for RZ/T2N SystemRAM)
LOADER_SIZE_MAX_RZT2M_BTCM   = 0x0000E000  # Maximum size of loader program: 56 KByte (for RZ/T2M, RZ/T2ME BTCM)
LOADER_SIZE_MAX_RZT2L_BTCM   = 0x0000E000  # Maximum size of loader program: 56 KByte (for RZ/T2L BTCM)
LOADER_SIZE_MAX_RZN2L_BTCM   = 0x0001E000  # Maximum size of loader program: 120 KByte (for RZ/N2L BTCM)
LOADER_UNIT_SIZE             = 0x0200      # Unit size of loader program: 512 Byte
LOADER_BOOT_KEY_AREA         = 0x0200      # Key area size for image decryption during secure boot for Loader
USERAPP_BOOT_KEY_AREA        = 0x0030      # Key area size for image decryption during secure boot for UserApp

# Sets the size of the device connected to the xSPI CS0 area.
XSPI0_CS0_SIZE_RZT2ME = 0x03FFFFFF
XSPI1_CS0_SIZE_RZT2ME = 0x00000000 # Not support.
XSPI0_CS0_SIZE_RZT2H  = 0x03FFFFFF
XSPI1_CS0_SIZE_RZT2H  = 0x00FFFFFF
XSPI0_CS0_SIZE_RZT2N  = 0x03FFFFFF
XSPI1_CS0_SIZE_RZT2N  = 0x00FFFFFF

# The setting for Select Second Boot CPU of RZ/T2H, RZ/N2H.
BOOTCPU_FLG_CR52       = 0x00000000       #RZ/T2H, RZ/N2H Cortex-R52
BOOTCPU_FLG_CA55       = 0x00000001       #RZ/T2H, RZ/N2H Cortex-A55

# Program base address in xSPI0 boot mode
# Minimum address where the program is located when xspi0 mode is specified
PROG_BASE_ADDR_XSPI0_RZT2H     = 0x4000004C
PROG_BASE_ADDR_XSPI0_RZT2N     = 0x4000004C
PROG_BASE_ADDR_XSPI0_RZT2M     = 0x6000004C
PROG_BASE_ADDR_XSPI0_RZT2L     = 0x6000004C
PROG_BASE_ADDR_XSPI0_RZN2L     = 0x6000004C

prg_base_addr_xspi0 = {
    'rzt2h_cr52':  PROG_BASE_ADDR_XSPI0_RZT2H,
    'rzt2h_ca55':  PROG_BASE_ADDR_XSPI0_RZT2H,
    'rzt2n_cr52':  PROG_BASE_ADDR_XSPI0_RZT2N,
    'rzt2n_ca55':  PROG_BASE_ADDR_XSPI0_RZT2N,
    'rzn2h_cr52':  PROG_BASE_ADDR_XSPI0_RZT2H,
    'rzn2h_ca55':  PROG_BASE_ADDR_XSPI0_RZT2H,
    'rzt2m':       PROG_BASE_ADDR_XSPI0_RZT2M,
    'rzt2me':      PROG_BASE_ADDR_XSPI0_RZT2M,
    'rzt2l':       PROG_BASE_ADDR_XSPI0_RZT2L,
    'rzn2l':       PROG_BASE_ADDR_XSPI0_RZN2L,
}

# Program base address in xSPI1 boot mode
# Minimum address where the program is located when xspi1 mode is specified
PROG_BASE_ADDR_XSPI1_RZT2H = 0x5000004C
PROG_BASE_ADDR_XSPI1_RZT2N = 0x5000004C
PROG_BASE_ADDR_XSPI1_RZT2M = 0x6800004C
PROG_BASE_ADDR_XSPI1_RZT2L = 0x6800004C
PROG_BASE_ADDR_XSPI1_RZN2L = 0x6800004C

prg_base_addr_xspi1 = {
    'rzt2h_cr52':  PROG_BASE_ADDR_XSPI1_RZT2H,
    'rzt2h_ca55':  PROG_BASE_ADDR_XSPI1_RZT2H,
    'rzt2n_cr52':  PROG_BASE_ADDR_XSPI1_RZT2N,
    'rzt2n_ca55':  PROG_BASE_ADDR_XSPI1_RZT2N,
    'rzn2h_cr52':  PROG_BASE_ADDR_XSPI1_RZT2H,
    'rzn2h_ca55':  PROG_BASE_ADDR_XSPI1_RZT2H,
    'rzt2m':       PROG_BASE_ADDR_XSPI1_RZT2M,
    'rzt2me':      PROG_BASE_ADDR_XSPI1_RZT2M,
    'rzt2l':       PROG_BASE_ADDR_XSPI1_RZT2L,
    'rzn2l':       PROG_BASE_ADDR_XSPI1_RZN2L,
}

# Program base address in bus boot mode
# Minimum address where the program is located when bus mode is specified
PROG_BASE_ADDR_BUS_RZT2M       = 0x7000004C
PROG_BASE_ADDR_BUS_RZT2L       = 0x7000004C
PROG_BASE_ADDR_BUS_RZN2L       = 0x7000004C

prg_base_addr_bus = {
    'rzt2h_cr52':  0x00000000, # Not support.
    'rzt2h_ca55':  0x00000000, # Not support.
    'rzt2n_cr52':  0x00000000, # Not support.
    'rzt2n_ca55':  0x00000000, # Not support.
    'rzn2h_cr52':  0x00000000, # Not support.
    'rzn2h_ca55':  0x00000000, # Not support.
    'rzt2m':       PROG_BASE_ADDR_BUS_RZT2M,
    'rzt2me':      PROG_BASE_ADDR_BUS_RZT2M,
    'rzt2l':       PROG_BASE_ADDR_BUS_RZT2L,
    'rzn2l':       PROG_BASE_ADDR_BUS_RZN2L,
}

# Loader program max
loader_prg_max = {
    'rzt2h_cr52':  LOADER_SIZE_MAX_RZT2H_BTCM,
    'rzt2n_cr52':  LOADER_SIZE_MAX_RZT2N_SYSRAM, # Only RZ/T2N CR52 can be deployed in both BTCM and SYSRAM.
    'rzn2h_cr52':  LOADER_SIZE_MAX_RZT2H_BTCM,
    'rzt2h_ca55':  LOADER_SIZE_MAX_RZT2H_SYSRAM,
    'rzt2n_ca55':  LOADER_SIZE_MAX_RZT2N_SYSRAM,
    'rzn2h_ca55':  LOADER_SIZE_MAX_RZT2H_SYSRAM,
    'rzt2m':       LOADER_SIZE_MAX_RZT2M_BTCM,
    'rzt2me':      LOADER_SIZE_MAX_RZT2M_BTCM,
    'rzt2l':       LOADER_SIZE_MAX_RZT2L_BTCM,
    'rzn2l':       LOADER_SIZE_MAX_RZN2L_BTCM,
}

# CS0 size for RZ/T2ME
cs0_size_rzt2me = {
    'xspi0':       XSPI0_CS0_SIZE_RZT2ME,
    'xspi1':       XSPI1_CS0_SIZE_RZT2ME,
}

# CS0 size for RZ/T2H, RZ/N2H
cs0_size_rzt2h_rzn2h = {
    'xspi0':       XSPI0_CS0_SIZE_RZT2H,
    'xspi1':       XSPI1_CS0_SIZE_RZT2H,
}

# CS0 size for RZ/T2N
cs0_size_rzt2n = {
    'xspi0':       XSPI0_CS0_SIZE_RZT2N,
    'xspi1':       XSPI1_CS0_SIZE_RZT2N,
}

# Second boot CPU config for RZ/T2H, RZ/N2H
bootcpu_cfg_rzt2h_rzn2h = {
    'rzt2h_cr52':  BOOTCPU_FLG_CR52,
    'rzn2h_cr52':  BOOTCPU_FLG_CR52,
    'rzt2h_ca55':  BOOTCPU_FLG_CA55,
    'rzn2h_ca55':  BOOTCPU_FLG_CA55,
}

# Device type
device_type = {
('rzt2h', 'cr52')   :  'rzt2h_cr52',
('rzt2h', 'ca55')   :  'rzt2h_ca55',
('rzt2n', 'cr52')   :  'rzt2n_cr52',
('rzt2n', 'ca55')   :  'rzt2n_ca55',
('rzn2h', 'cr52')   :  'rzn2h_cr52',
('rzn2h', 'ca55')   :  'rzn2h_ca55',
('rzt2m', 'cr52')   :  'rzt2m',
('rzt2me', 'cr52')  :  'rzt2me',
('rzt2l', 'cr52')   :  'rzt2l',
('rzn2l', 'cr52')   :  'rzn2l',
}

def main():
    """Main processing.
    
    Parsing command line arguments.
    Generates parameter information. 
    
    Args:
        None
    
    Returns:
        None
    
    """
    # Command line argument perse.
    parser = set_cmdline_argparser()
    args = parser.parse_args()
    
    # Check whether the required common options are specified.
    if False == os.path.exists(args.i):
        ERRMSG(str.format(ERRMSG_OPENFAIL % (args.i)))
        exit(1)
    
    if re.match("^[0-9A-Fa-f]{8}$", args.src_addr) is None:
        ERRMSG(str.format(ERRMSG_ADDRFAIL % (args.src_addr)))
        exit(1)
    
    # Delete files at output path.
    if os.path.exists(args.o) is True:
        os.remove(args.o)
    
    # Generate Parameter.
    # (args.cmd is always set to 'loader' or 'userapp')
    if args.cmd == 'loader':
        # Check that the necessary options for each command are specified.
        if re.match("^[0-9A-Fa-f]{8}$", args.dest_addr) is None:
            ERRMSG(str.format(ERRMSG_ADDRFAIL % (args.dest_addr)))
            exit(1)
        
        # Get the value from the option.
        file_len = os.path.getsize(args.i)
        saddr = int(args.src_addr, 16)
        daddr = int(args.dest_addr, 16)
        
        # Check the target device from mpu and cpu.
        target_device = device_type.get((args.mpu, args.target_cpu))
        if None == target_device:
            ERRMSG(str.format(ERRMSG_CPUFAIL % (args.target_cpu)))
            exit(1)
        
        # Check file size.
        loader_prg_size_limit = loader_prg_max[target_device]
        
        if args.secureboot == True:
            loader_prg_size_limit -=  LOADER_CERT_LEN
        
        if loader_prg_size_limit < file_len:
            ERRMSG(str.format(ERRMSG_SIZEFAIL % (args.i)))
            exit(1)
        
        if (file_len % LOADER_UNIT_SIZE) != 0:
            ERRMSG(str.format(ERRMSG_UNITSIZE % (args.i)))
            exit(1)
        
        if target_device == 'rzt2n_cr52':
            # Only RZ/T2N CR52 can be deployed in both BTCM and SYSRAM.
            # So, if the file size is larger than the maximum size of BTCM and within the maximum size of SYSRAM, output a warning.
            loader_prg_size_limit_for_rzt2n_cr52_btcm = LOADER_SIZE_MAX_RZT2N_BTCM
            
            if args.secureboot == True:
                loader_prg_size_limit_for_rzt2n_cr52_btcm = LOADER_SIZE_MAX_RZT2N_BTCM - LOADER_CERT_LEN
            
            if loader_prg_size_limit_for_rzt2n_cr52_btcm < file_len:
                LOGMSG(str.format(NOTICE_LIMIT_SIZE_T2N_CR52_BTCM % (file_len)))
                
        # Confirmation of option specification when RZ/T2H, RZ/T2N or RZ/N2H is specified.
        if target_device == 'rzt2h_cr52' or target_device == 'rzt2h_ca55' or target_device == 'rzt2n_cr52' or target_device == 'rzt2n_ca55' or target_device == 'rzn2h_cr52'or target_device == 'rzn2h_ca55':
            # Bus boot mode cannot be specified.
            if args.mode == 'bus':
                ERRMSG(str.format(ERRMSG_MODEFAIL % (args.mode)))
                exit(1)
        
        # Check for secure boot.
        if args.secureboot == True:
            # Check if the start address fits within the 32-bit address space.
            if WORD_BYTES_MAX_NUM <= (daddr + LOADER_CERT_LEN - 1):
                ERRMSG(str.format(ERRMSG_ADDRFAIL % (args.dest_addr)))
                exit(1)
            
            LOGMSG(str.format(NOTICE_DESTADDR % (args.dest_addr)))
        
        # Generate Parameters for the Loader.
        loader_param = generate_loader_parameter(saddr, daddr, file_len, target_device, args.mode, args.secureboot, args.use_commonkey4)
        
        # Check the flag to combine the program with the parameter information generation.
        if args.concat_loader == True:
            loader_param = combine_program_data(saddr, args.mode, target_device, args.i, loader_param)
            if loader_param is None:
                ERRMSG(str.format(ERRMSG_ADDRFAIL % (args.src_addr)))
                exit(1)
        
        # Write parameters.
        with open(args.o, 'bw') as outfile:
            outfile.write(loader_param)
        
    else:  # args.cmd == 'userapp'
        # Check that the necessary options for each command are specified.
        if re.match("^[0-9A-Fa-f]{8}$", args.app_start_addr) is None:
            ERRMSG(str.format(ERRMSG_ADDRFAIL % (args.app_start_addr)))
            exit(1)
        
        # Get the value from the option.
        file_len = os.path.getsize(args.i)
        saddr = int(args.src_addr, 16)
        aaddr = int(args.app_start_addr, 16)
        
        # Generate Parameters for the User application.
        userapp_param = generate_userapp_parameter(saddr, aaddr, file_len, args.secureboot, args.use_commonkey4)
        
        # Write parameters.
        with open(args.o, 'bw') as outfile:
            outfile.write(userapp_param)
        
    exit(0)

def set_cmdline_argparser():
    """Command line argument parsing settings.
    
    Set valid commands and options on the command line.
    
    Args:
        None
    
    Returns:
        parser - ArgumentParser object
    
    """
    DESC_TOOL       = 'Tool to generate parameter information'
    DESC_LOADER     = 'Generate Parameters for the Loader'
    DESC_USERAPP    = 'Generate Parameters for the User application'
    
    METAVER_ADDR    = '<Address>'
    METAVER_I       = '<ProgramFile>'
    METAVER_O       = '<ParamInfoFile>'
    METAVER_MODE    = '<BootMode>'
    METAVER_MPU     = '<TargetMPU>'
    METAVER_CPU     = '<TargetCPU>'
    
    HELP_CMD        = 'Specify the generation command'
    HELP_SADDR      = 'Specify the flash address where the target program will be written'
    HELP_DADDR      = 'Specify the RAM address where the target program will be expanded'
    HELP_APP_ADDR   = 'Specify the start address of the user application program'
    HELP_MODE       = 'Specify the target boot mode (choose from xspi0, xspi1 or bus)'
    HELP_LOADERI    = 'Specify the path of the loader program file'
    HELP_USERI      = 'Specify the path of the user application program file'
    HELP_O          = 'Specify the path of the parameter information file to be output'
    HELP_ADD        = 'Specify when combining parameter information generation and program'
    HELP_SEC        = '(For secure boot) Specify when creating parameter information for secure'
    HELP_COMKEY4    = '(For secure boot) Specify this when using the key stored in the flash for image decryption'
    HELP_MPU        = 'Specify the target MPU'
    HELP_CPU        = 'Specify the target CPU'
    
    # Set command line parser.
    parser = argparse.ArgumentParser(description=DESC_TOOL)
    subparsers = parser.add_subparsers(dest='cmd',required=True, help=HELP_CMD)
    parser_loader = subparsers.add_parser('loader', description=DESC_LOADER)
    parser_userapp = subparsers.add_parser('userapp', description=DESC_USERAPP)
    
    # Set the argument of the "loader" command.
    parser_loader.add_argument('--mpu', choices=['rzt2h', 'rzt2n', 'rzn2h', 'rzt2m', 'rzt2me', 'rzt2l', 'rzn2l'], metavar=METAVER_MPU, required=True, help=HELP_MPU)
    parser_loader.add_argument('--target_cpu', choices=['cr52', 'ca55'], metavar=METAVER_CPU, required=True, help=HELP_CPU)
    parser_loader.add_argument('--src_addr', metavar=METAVER_ADDR, required=True, help=HELP_SADDR)
    parser_loader.add_argument('--dest_addr', metavar=METAVER_ADDR, required=True, help=HELP_DADDR)
    parser_loader.add_argument('--mode', choices=['xspi0', 'xspi1', 'bus'], metavar=METAVER_MODE, required=True, help=HELP_MODE)
    parser_loader.add_argument('-i', metavar=METAVER_I, required=True, help=HELP_LOADERI)
    parser_loader.add_argument('-o', metavar=METAVER_O, required=True, help=HELP_O)
    parser_loader.add_argument('--concat_loader', action='store_true', help=HELP_ADD)
    parser_loader.add_argument('--secureboot', action='store_true', help=HELP_SEC)
    parser_loader.add_argument('--use_commonkey4', action='store_true', help=HELP_COMKEY4)
    
    # Set the argument of the "userapp" command.
    parser_userapp.add_argument('--src_addr', metavar=METAVER_ADDR, required=True, help=HELP_SADDR)
    parser_userapp.add_argument('--app_start_addr', metavar=METAVER_ADDR, required=True, help=HELP_APP_ADDR)
    parser_userapp.add_argument('-i', metavar=METAVER_I, required=True, help=HELP_USERI)
    parser_userapp.add_argument('-o', metavar=METAVER_O, required=True, help=HELP_O)
    parser_userapp.add_argument('--secureboot', action='store_true', help=HELP_SEC)
    parser_userapp.add_argument('--use_commonkey4', action='store_true', help=HELP_COMKEY4)
    
    return parser

def generate_loader_parameter(src_addr, dest_addr, prg_len, target_device, mode, sec_flg, comkey4_flg):
    """Generate Parameters for the Loader.
    
    Generates loader parameter information from the values
    specified in the command and internal constants.
    
    Args:
        src_addr (int) - Src address
        dest_addr (int) - Dest address
        prg_len (int) - Program size
        target_device (str) - Target device
        mode (str) - Boot mode
        sec_flg (bool) - Flag for secureboot
        comkey4_flg (bool) - Flag for commomkey4
    
    Returns:
        Parameters for the Loader (bytes)
    
    """
    loader_param = b''
    
    # Add a fixed value included in the parameter information for the loader.
    loader_param += CACHE_FLG.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    if mode == 'bus':
        # For Bus Boot Mode.
        loader_param += CS0BCR.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        loader_param += CS0WCR.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        loader_param += bytes.fromhex('00' * 4)
        
    else:  # args.mode == 'xspi0' or 'xspi1'
        # For xSPI Boot Mode.
        loader_param += WRAPCFG.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        loader_param += COMCFG.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        loader_param += BMCFG.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
        
    loader_param += INIT_FLG.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add flash write destination address.
    loader_param += src_addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add program file size.
    if sec_flg == True:
        # Calculate secure boot image file size for secure.
        if (prg_len % SEC_IMAGE_UNIT_SIZE) != 0:
            prg_len += SEC_IMAGE_UNIT_SIZE - (prg_len % SEC_IMAGE_UNIT_SIZE)
        
        prg_len += LOADER_CERT_LEN
        
        if comkey4_flg == True:
            prg_len += LOADER_BOOT_KEY_AREA
        
    loader_param += prg_len.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add RAM expansion destination address.
    loader_param += dest_addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add a fixed value included in the parameter information for the loader.
    if mode == 'bus':
        # For Bus Boot Mode.
        loader_param += bytes.fromhex('00' * 40)
        
    else:  # args.mode == 'xspi0' or 'xspi1'
        # For xSPI Boot Mode.
        loader_param += bytes.fromhex('00' * 8)
        
        # Check the target_device.
        if target_device == 'rzt2h_cr52' or target_device == 'rzt2h_ca55' or target_device == 'rzn2h_cr52'or target_device == 'rzn2h_ca55':
            # Set parameters for RZ/T2H or RZ/N2H.
            loader_param += cs0_size_rzt2h_rzn2h[mode].to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += LIOCFGCS0.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += PLL0_SSC_CTR.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += bootcpu_cfg_rzt2h_rzn2h[target_device].to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += bytes.fromhex('00' * 12)
            loader_param += ACCESS_SPEED.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            
        elif target_device == 'rzt2n_cr52' or target_device == 'rzt2n_ca55':
            # Set parameters for RZ/T2N.
            loader_param += cs0_size_rzt2n[mode].to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += LIOCFGCS0.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += PLL0_SSC_CTR.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += bytes.fromhex('00' * 16)
            loader_param += ACCESS_SPEED.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            
        elif target_device == 'rzt2me':
            # Set parameters for RZ/T2ME.
            loader_param += cs0_size_rzt2me[mode].to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += LIOCFGCS0.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += bytes.fromhex('00' * 20)
            loader_param += ACCESS_SPEED.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            
        else:
            # Set parameters for RZ/T2M, RZ/T2L, RZ/N2L.
            loader_param += CSSCTL.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += LIOCFGCS0.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            loader_param += bytes.fromhex('00' * 20)
            loader_param += ACCESS_SPEED.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
            
    # Calculate checksum.
    mv = memoryview(loader_param).cast('H')
    sum = 0
    
    for item in mv:
        sum += item
    sum %= (WORD_BYTES_MAX_NUM + 1)
    
    loader_param += sum.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    return loader_param

def generate_userapp_parameter(src_addr, app_start_addr, prg_len, sec_flg, comkey4_flg):
    """Generate Parameters for the User application.
    
    Generates userapp parameter information from the value
    specified in the command.
    
    Args:
        src_addr (int) - Src address
        app_start_addr (int) - App start address
        prg_len (int) - Program size
        sec_flg (bool) - Flag for secureboot
        comkey4_flg (bool) - Flag for commomkey4
    
    Returns:
        Parameters for the User application (bytes)
    
    """
    userapp_param = b''
    
    # Add flash write destination address.
    userapp_param += src_addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add program file size.
    if sec_flg == True:
        if (prg_len % SEC_IMAGE_UNIT_SIZE) != 0:
            prg_len += SEC_IMAGE_UNIT_SIZE - (prg_len % SEC_IMAGE_UNIT_SIZE)
        
        prg_len += USERAPP_CERT_LEN
        
        if comkey4_flg == True:
            prg_len += USERAPP_BOOT_KEY_AREA
        
    userapp_param += prg_len.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Add User application start address.
    userapp_param += app_start_addr.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    # Calculate checksum.
    mv = memoryview(userapp_param).cast('H')
    sum = 0
    
    for item in mv:
        sum += item
    sum %= (WORD_BYTES_MAX_NUM + 1)
    
    userapp_param += sum.to_bytes(WORD_BYTES, ENDIAN_LITTLE)
    
    return userapp_param

def combine_program_data(src_addr, mode, target_device, prg_path, param):
    """Combine program data.
    
    Combine program data with parameter information.
    
    Args:
        src_addr (int) - Src address
        mode (str) - Boot mode
        target_device (str) - Target device
        prg_path (str) - Program file path
        param (bytes) - Parameter info
    
    Returns:
        Parameter info + program data (bytes)
    
    """
    if mode == 'bus':
        padding_size = src_addr - prg_base_addr_bus[target_device]
        
    elif mode == 'xspi0':
        padding_size = src_addr - prg_base_addr_xspi0[target_device]
        
    else:  # mode == 'xspi1'
        padding_size = src_addr - prg_base_addr_xspi1[target_device]
        
    # Check padding size.
    if 0 > padding_size:
        return None
        
    param += bytes.fromhex(PADDING_VAL * padding_size)
    
    # Open input file and read data.
    with open(prg_path, mode='rb') as f:
        file_data = f.read()
       
    # Add data read from file.
    param += file_data
    
    return param

if __name__=='__main__':
    main()

