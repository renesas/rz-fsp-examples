/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the typical use of the WDT HAL module APIs in collaboration with the GTM timer
    interrupt and the PMOD LED/Button.
    On user input, the project initializes the WDT (timeout configured to 0.175 seconds) and starts a GTM timer that
    expires every 50 milliseconds.
    Each GTM expiry refreshes the WDT counter and toggles LD0 on the PMOD LED.
    The WDT refresh happens every 50 milliseconds, but the counter refresh status is printed on the J-Link RTT Viewer
    once every 3 seconds.
    If the user presses BTN0 on the PMOD BUTTON, the WDT refresh is stopped; the WDT then times
    out, resets the MCU, and the J-Link connection will be disconnected due to the MCU reset.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module.
    1 x PMOD BUTTON module.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+========================================+
        | Board setting | RZ/G3E Evaluation Board Kit            |
        +===============+========================================+
        | Module board  | BOOT-1            : OFF                |
        |               | BOOT-2            : OFF                |
        |               | BOOT-3            : OFF                |
        |               | BOOT-4            : OFF                |
        |               | BOOT-5            : OFF                |
        |               | BOOT-6            : OFF                |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | PMOD_PWR_SEL     : Jumper connects 1-3 |
        |               |                  : Jumper connects 2-4 |
        |               | SW_GPIO_OPT_SEL  : 2-3                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD0-2A pins 7–12 on the SMARC Carrier Board to the PMOD BUTTON for button input. 
    (4) Connect PMOD1-3A pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control. 

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x080076a0
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).