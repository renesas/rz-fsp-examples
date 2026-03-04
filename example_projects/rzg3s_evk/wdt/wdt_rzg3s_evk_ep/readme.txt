/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the use of WDT module with the collaboration of module IRQ, GTM, WDT
    and button/led.
    The operation of this example is as below:
        Start GTM timer having a callback every 1 second, to refresh WDT and blink the led (LD2 on Pmod LED).
        The WDT timeout is configured to 2 seconds.
        If the button BTN3 (on PMOD button) is pressed, the WDT reset will issued.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3S-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB PD Charger Anker PowerPort III 65W Pod.
    1 x PMOD LED module.
    1 x PMOD button module.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+========================================+
        | Board setting | RZ/G3S Evaluation Board Kit            |
        +===============+========================================+
        | Module board  | SW_CONFIG-1      : OFF                 |
        |               | SW_CONFIG-2      : OFF                 |
        |               | SW_CONFIG-3      : ON                  |
        |               | SW_CONFIG-4      : OFF                 |
        |               | SW_CONFIG-5      : OFF                 |
        |               | SW_CONFIG-6      : OFF                 |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               |                                        |
        |               | PMOD_PWR_SEL     : Jumper connects 1-3 |
        |               |                  : Jumper connects 2-4 |
        |               |                                        |
        |               | SW_PMOD0_PWR_SLP : 5-6                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1_3A pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control.
    (4) Connect PMOD0_2A pins 7–12 on the SMARC Carrier Board to the PMOD button for button input control.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x00027028
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).