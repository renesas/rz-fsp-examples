/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of GPT HAL module APIs.
    User has the provision to input value as per displayed menu through JLinkRTTViewer to select different GPT 
    supported modes (Periodic, PWM, One-Shot).
    In periodic mode, user can enter the time period within the permitted ranges to change the frequency of
    the user Pmod LED.
    In PWM mode, user can enter the duty cycle within the specified range to adjust the intensity of the user Pmod LED.
    In One-Shot mode, output will be displayed on JlinkRTTViewer.
    Any failure will also be displayed on JlinkRTTViewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module.

4. Hardware Configurations:
    (1) Boot Mode:
        CA55 coldboot (SCIF boot).
    (2) Set for DIP switches and jumpers as follow.

        +================+================================+
        | Board setting  | RZ/V2H EVK                     |
        +================+================================+
        | Module board   | DSW1-1  : ON                   |
        |                | DSW1-2  : OFF                  |
        |                | DSW1-3  : ON                   |
        |                | DSW1-4  : OFF                  |
        |                | DSW1-5  : ON                   |
        |                | DSW1-6  : OFF                  |
        |                | DSW1-7  : ON                   |
        |                | DSW1-8  : OFF                  |
        |                |                                |
        |                | DSW2-1  : OFF                  |
        |                | DSW2-2  : OFF                  |
        |                | DSW2-3  : OFF                  |
        |                | DSW2-4  : OFF                  |
        |                | DSW2-5  : OFF                  |
        |                | DSW2-6  : OFF                  |
        |                |                                |
        |                | DSW3-1  : ON                   |
        |                | DSW3-2  : ON                   |
        |                | DSW3-3  : ON                   |
        |                | DSW3-4  : ON                   |
        |                | DSW3-5  : ON                   |
        |                | DSW3-6  : ON                   |
        |                | DSW3-7  : ON                   |
        |                | DSW3-8  : ON                   |
        |                |                                |
        +----------------+--------------------------------+

5. Hardware Connections:
    (1) Connect the CN13 on the RZ/V2H Evaluation Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD LED to Pmod Type1/1A(CN1) (pin 7-12) on RZ/V2H EVK Expansion Board.

Note:
1. Timer period range in millisecond as per Example Project configuration:
   RZ/V2H   --      (0 to 21474) ms 
2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08184d8c
3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).