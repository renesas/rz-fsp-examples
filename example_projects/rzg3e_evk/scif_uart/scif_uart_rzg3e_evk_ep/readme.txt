/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the SCIF_UART HAL module APIs. The project initializes the UART
    with Baud rate of 115200 bps and GTM module. Using a Terminal Program (like Tera Term) user can provide a value & 
    press enter key to set the period of the PMOD LED signal (on LD0). The range of input values are displayed on the
    J-Link RTT Viewer. Any failure will also be displayed using J-Link RTT Viewer. To see user input values on
    Serial terminal, enable local echo option.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.
    1 x PMOD LED module.

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
        |               | PMOD_PWR_SEL     : Jumper connects 2-4 |
        |               | SW_GPIO_OPT_SEL  : 2-3                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect serial cable to SER3_UART port on SMARC Carrier Board to PC.
    (4) Connect PMOD1-6A pins 1–6 on the SMARC Carrier Board to the PMOD LED for LED output control. 

6. Verifying Operations:
    Tera Term setup:
    - Setup -> Serial port -> Speed: 115200
    - Setup -> Terminal -> Enable "Local echo"

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08007228
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
