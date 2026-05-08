/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the SCI_B_UART HAL module APIs.
    The project initializes the UART with baud rate of 115200 bps and GTM module.
    Using a Terminal Program (like Tera Term) user can provide a value & press enter key to set the period 
    of the PMOD LED signal.
    The range of input values are displayed on the J-Link RTT Viewer.
    Any failure will also be displayed using J-Link RTT Viewer.
    To see user input values on Serial terminal, enable local echo option.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module (e.g., https://digilent.com/reference/pmod/pmodled/start).
    1 x PMOD UART module (e.g., https://digilent.com/reference/pmod/pmodusbuart/start).
    1 x USB Micro-B cable.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.
        +===============+========================================+
        | Board setting | RZ/G3E Evaluation Board Kit            |
        +===============+========================================+
        | Module board  | BOOT-1           : OFF                 |
        |               | BOOT-2           : OFF                 |
        |               | BOOT-3           : OFF                 |
        |               | BOOT-4           : OFF                 |
        |               | BOOT-5           : OFF                 |
        |               | BOOT-6           : OFF                 |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | PMOD PWR SELECT  : Jumper connects 1-3 |
        |               |                    Jumper connects 2-4 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1_6A (PIN1-6) on the SMARC Carrier Board to the PMOD LED for LED output control. 
    (4) Connect PMOD UART and SMARC Carrier Board as follows.
        J2 (PMOD UART)            PMOD1_3A (SMARC Carrier Board)
        ---                          -------
        |1|---RTS                    |1   7|
        |2|---RXD  --->  SCIFx_TXD---|2   8|
        |3|---TXD  --->  SCIFx_RXD---|3   9|
        |4|---CTS                    |4  10|
        |5|---GND  --->  GND pin  ---|5  11|
        |6|---VCC  --->  VCC pin  ---|6  12|
        ---                          -------

        Connect J1 on PMOD UART to PC using a USB Micro-B cable.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08008a60

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).

3. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

   Enable local echo option: Setup -> Terminal -> Enable "Local echo"