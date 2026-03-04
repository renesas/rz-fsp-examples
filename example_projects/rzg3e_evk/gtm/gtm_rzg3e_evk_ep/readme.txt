/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the functionality of GTM in Periodic mode and One-shot mode. On providing any
    input on the J-Link RTT Viewer, GTM channel 2 starts in One-shot mode. GTM channel 1 starts in Periodic mode when
    GTM channel 2 expires. Timer in Periodic mode expires periodically at a time period specified by user and toggles
    the LD0 on PMOD LED.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module

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
        |               | PMOD PWR SELECT  : Jumper connects 2-4 |
        |               | SW_GPIO_OPT_SEL  : 2-3                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1-3A pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control. 

Note:
1. User is expected to enter data of size not exceeding above 15 bytes.
   Operation is not guaranteed for any user input value other than integer, char (i.e. float, special char)
   through RTT.
2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08007348
3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
