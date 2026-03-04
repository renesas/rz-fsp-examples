/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CANFD running on Renesas RZ MPUs using Channel 0 and Channel 1 on RZ/V 
    board. On pressing any key on the Terminal Emulator, data is transmitted from one Channel to other. On 1st 
    transmission, Channel 0 transmits data to Channel 1. Channel 1 displays the received data.On 2nd transmission, 
    Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data. Then, Channel 0 changes 
    CAN frame to CANFD frame and updates data. On 3rd transmission, Channel 0 transmits updated data to Channel 1. 
    Then, Channel 1 changes CAN frame to CANFD frame and updates data to transmits back to
    Channel 0 as ACK. On 4th transmission, Channel 1 transmits updated data to Channel 0 as
    ACK. Channel 0 displays the received data

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2L-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED.
    1 x CAN cable.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).
    (2) Set for DIP switches and jumpers as follow.

        +================+================================+
        | Board setting  | RZ/V2L Evaluation Kit (SMARC)  |
        +================+================================+
        | Module board   | SW1-1  : OFF                   |
        |                | SW1-2  : Don't care            |
        |                |                                |
        +----------------+--------------------------------+
        | Carrier board  | SW1    : Don't care            |
        |                | SW2    : Don't care            |
        |                | SW3    : Don't care            |
        |                | SW4    : Don't care            |
        |                | SW5    : Don't care            |
        |                | SW6    : Don't care            |
        |                | SW7    : 1-2                   |
        |                | SW8    : 1-2                   |
        |                | SW11-1 : OFF                   |
        |                | SW11-2 : OFF                   |
        |                | SW11-3 : OFF                   |
        |                | SW11-4 : ON                    |
        |                |                                |
        |                | CN4    : Jumper connects 1-3   |
        |                |          Jumper connects 2-4   |
        +----------------+--------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control.
    (4) Connect CN15 to CN16 (CAN_H, CAN_L, GND) for CAN communication by using CAN cable.
    
Note:
1. Segger RTT block address may be needed to download and observe example project operation
   using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x60110a10

2. If an example project is modified, compiled, and downloaded please find the block address
   (for the variable in RAM called _SEGGER_RTT) in .map file generated in the build
   configuration folder (Debug/Release).

3. The RZ/V2L-EVK must have IC15 (CAN transceiver) populated on the carrier board to support CAN communication.

4. a. When CAN transmission operation is in progress:
       LEDs will be turned ON from LD0 to LD3 then turned OFF sequentially from LD0 to LD3.
    b. When CAN transmission operation is successful:
       LEDs LD0 through LD3 toggle continuously.
    c. When any error occurs in CAN operation, all LEDs will be turned ON.

5. The user must enter data of no more than 15 bytes.