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
    RZ supported boards: RZ/V2N-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod LED.

4. Hardware Configurations:
    (1) Boot Mode:
        CA55 coldboot (SCIF boot).
    (2) Set for DIP switches and jumpers as follow.

        +================+================================+
        | Board setting  | RZ/V2N EVK                     |
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
        |                | DSW2    : Default              |
        |                |                                |
        |                | DSW3    : Default              |
        |                |                                |
        +----------------+--------------------------------+

5. Hardware Connections:
    (1) Connect the CN13 on the RZ/V2N Evaluation Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD LED to lower row PMOD Type1/1A on Expansion Board.

Note:

1. Timer period range in millisecond as per Example Project configuration:
    (1) RZ/V2N   --      (0 to 21474) ms

2. User is expected to enter data of size not exceeding above 15 bytes.
   Operation is not guaranteed for any user input value other than integer (i.e. float, char, special char) through RTT.
   
3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x0800aca0

4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
