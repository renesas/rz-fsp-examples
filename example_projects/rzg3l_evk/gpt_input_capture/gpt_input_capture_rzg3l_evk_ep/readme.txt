/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The Example project demonstrates the functionality of GPT Input Capture module. GPT4 is used to 
    generate periodic pulses of 500msec duration and provided as input to GPT Input Capture (GPT0).
    GPT0 counts the event pulse received at its input.
    Based on the period and capture event, the time period of pulse is calculated and displayed on RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 9.44
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3L-EVK.
    1 x Renesas RZ board.
    1 x USB Type-C cable.
    1 x Jumper wire.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+============================================+
        | Board setting | RZ/G3L Evaluation Kit (SMARC)              |
        +===============+============================================+
        | Module board  | SW5-1  : OFF                               |
        |               | SW5-2  : OFF                               |
        |               | SW5-3  : OFF                               |
        |               | SW5-4  : OFF                               |
        |               | SW5-5  : OFF                               |
        |               | SW5-6  : OFF                               |
        +---------------+--------------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                     |
        |               | SW_MODE-2        : ON                      |
        |               | SW_MODE-3        : OFF                     |
        |               | SW_MODE-4        : ON                      |
        |               | PMOD_PWR_SEL     : Jumper connects 2-4     |
        +---------------+--------------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C_PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for 
        board power supply.
    (2) Connect the J-Link Base and the connector (CN1) on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1_3A PIN10 and PMOD1_6A PIN3 on SMARC Carrier Board using a jumper wire.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x0002d108

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
