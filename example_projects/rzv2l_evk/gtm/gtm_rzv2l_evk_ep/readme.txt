/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the functionality of GTM in Periodic mode and One-Shot mode. On providing any 
    input on the RTT Viewer, GTM channel 2 starts in One-Shot mode. GTM channel 1 starts in Periodic mode when GTM 
    channel 2 expires. Timer in Periodic mode expires periodically at a time period specified by user and toggles the 
    LED on Pmod LED.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2L-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod LED.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).
    (2) Set for DIP switches and jumpers as follow.

        +===============+===============================+
        | Board setting | RZ/V2L Evaluation Kit (SMARC) |
        +===============+===============================+
        | Module board  | SW1-1  : OFF                  |
        |               | SW1-2  : Don't care           |
        +---------------+-------------------------------+
        | Carrier board | SW1    : Don't care           |
        |               | SW2    : Don't care           |
        |               | SW3    : Don't care           |
        |               | SW4    : Don't care           |
        |               | SW5    : Don't care           |
        |               | SW6    : Don't care           |
        |               | SW7    : Don't care           |
        |               | SW8    : Don't care           |
        |               | SW11-1 : OFF                  |
        |               | SW11-2 : OFF                  |
        |               | SW11-3 : OFF                  |
        |               | SW11-4 : ON                   |
        |               |                               |
        |               | CN4    : Jumper connects 1-3  |
        |               |          Jumper connects 2-4  |
        +---------------+-------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control (LED LD2).

Note:
1. User is expected to enter data of size not exceeding above 15 bytes.
   Operation is not guaranteed for any user input value other than integer, char (i.e. float, special char) through RTT.

2. Segger RTT block address may be needed to download and observe example project operation
   using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x601105b0

3. If an example project is modified, compiled, and downloaded please find the block address
   (for the variable in RAM called _SEGGER_RTT) in .map file generated in the build
   configuration folder (Debug/Release).
