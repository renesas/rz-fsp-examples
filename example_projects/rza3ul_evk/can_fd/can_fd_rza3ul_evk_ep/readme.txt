/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CAN-FD running on Renesas RZ MPUs using channel 0 and channel 1 on board. 
    On pressing any key on the Terminal Emulator, data is transmitted from one channel to other. On 1st transmission,
    Channel 0 transmits data to Channel 1. Channel 1 displays the received data. On 2nd transmission, Channel 1
    transmits updated data to Channel 0 as ACK. Channel 0 displays the received data then Channel 0 changes CAN frame to
    CANFD frame and updates data.On 3rd transmission, Channel 0 transmits updated data to Channel 1. Channel 1 displays
    the received data then Channel 1 changes CAN frame to CANFD frame and updates data to transmits back to Channel 0 as
    ACK. On 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.
    1 x PMOD LED module.

    RZ supported boards:
        RZ/A3UL Evaluation Board Kit QSPI Edition  (Exec with DDR SDRAM).
        RZ/A3UL Evaluation Board Kit QSPI Edition  (eXecute-In-Place).
        RZ/A3UL Evaluation Board Kit OCTAL Edition (eXecute-In-Place).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).

    (2) Set for DIP switches and jumpers as follow:
        +===============+====================================+
        | Board setting | RZ/A3UL Evaluation Board Kit       |
        +===============+====================================+
        | Module board  | SW1-1   : OFF                      |
        |               | SW1-2   : OFF                      |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Default                  |
        |               | SW2     : Default                  |
        |               | SW3     : Default                  |
        |               | SW4     : Default                  |
        |               | SW5     : Default                  |
        |               | SW6     : Default                  |
        |               | SW7     : 1-2                      |
        |               | SW8     : 1-2                      |
        |               |                                    |
        |               | SW11-1  : OFF                      |
        |               | SW11-2  : OFF                      |
        |               | SW11-3  : OFF                      |
        |               | SW11-4  : ON                       |
        |               |                                    |
        |               | CN4     : Jumper connects 1-3      |
        |               |           Jumper connects 2-4      |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMRAC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect the CN14 port on the Smarc Carrier Board to the PC using a USB Type-microB cable for serial terminal
        communication (Tera Term).
    (4) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control.
    (5) Connect CN15 to CN16 (CAN_H, CAN_L, GND) for CAN communication.

Note:
    The RZ/A3UL-EVK must have IC15 (CAN transceiver) populated on the carrier board to support CAN communication.