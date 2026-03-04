/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CAN-FD running on Renesas RZ MPUs using channel 0 and channel 1 on board.
    On pressing any key on the Terminal Emulator, data is transmitted from one channel to other. On 1st transmission,
    Channel 0 transmits data to Channel 1. Channel 1 displays the received data. On 2nd transmission, Channel 1
    transmits updated data to Channel 0 as ACK. Channel 0 displays the received data. Then, Channel 0 changes CAN frame
    to CANFD frame and updates data. On 3rd transmission, Channel 0 transmits updated data to Channel 1. Channel 1
    displays the received data. Then, Channel 1 changes CAN frame to CANFD frame and updates data to transmits back to
    Channel 0 as ACK. On 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the
    received data.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G2UL-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod LED.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).
    (2) Set for DIP switches and jumpers as follow.

        +================+=================================+
        | Board setting  | RZ/G2UL Evaluation Kit (SMARC)  |
        +================+=================================+
        | Module board   | SW1-1  : OFF                    |
        |                | SW1-2  : Don't care             |
        |                | SW1-3  : OFF                    |
        +----------------+---------------------------------+
        | Carrier board  | SW1    : Don't care             |
        |                | SW2    : Don't care             |
        |                | SW3    : Don't care             |
        |                | SW4    : Don't care             |
        |                | SW5    : Don't care             |
        |                | SW6    : Don't care             |
        |                | SW7    : 1-2                    |
        |                | SW8    : 1-2                    |
        |                | SW11-1 : OFF                    |
        |                | SW11-2 : OFF                    |
        |                | SW11-3 : OFF                    |
        |                | SW11-4 : ON                     |
        |                | CN4    : Jumper connects 1-3    |
        |                |          Jumper connects 2-4    |
        +----------------+---------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD0 PIN7-12 on SMARC Carrier Board and Pmod LED.
        Connect CN15 to CN16 (CAN_H, CAN_L, GND)

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x601109b8

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).