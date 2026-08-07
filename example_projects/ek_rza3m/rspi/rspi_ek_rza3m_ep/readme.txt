/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RSPI HAL module APIs.
    The project configure RSPI channels (channel 1 and channel 0) in Master and Slave mode.
    Once the module is initialised and the channels are configured,
    Master and Slave can transmit and receive data based on commands from user sent through Terminal Emulator.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    4 x jumper wires.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 4 (3.3-V Single or Quad serial NOR flash memory).

    (2) Set for DIP switches as follow to select boot mode 4 and peripheral pins:
        +===============+==========================+
        | Board setting | EK-RZ/A3M                |
        +===============+==========================+
        | Module board  | SW4-1  : OFF (Default)   |
        |               | SW4-2  : OFF (Default)   |
        |               | SW4-3  : OFF (Default)   |
        |               | SW4-4  : OFF (Default)   |
        |               | SW4-5  : OFF             |
        |               |                          |
        |               | SW5-1  : OFF             |
        |               | SW5-2  : OFF             |
        |               | SW5-3  : OFF (Default)   |
        |               | SW5-4  : OFF             |
        |               | SW5-5  : OFF (Default)   |
        +---------------+--------------------------+

    (3) The user needs to place jumper J9 on pins 2-3 to use on-board debug functionality.

5. Hardware Connections:
    (1) Connect the DEBUG1 port (J10) of the RZ board to the PC using a USB Type-C cable for board power supply, debugging,
        and serial terminal communication (Tera Term).
    (2) Connect pins with jumper on J1 of EK-RZ/A3M Board as follows:

        |    Master (J1)            |        Slave (J1)         |
        |---------------------------|---------------------------|
        |  P1_2|---RSPI1_CK    <----|---->  RSPI0_CK-----|P4_5  |
        |  P1_4|---RSPI1_MISO  <----|---->  RSPI0_MISO---|P5_1  |
        |  P1_3|---RSPI1_MOSI  <----|---->  RSPI0_MOSI---|P5_0  |
        |  P2_0|---RSPI1_SSL   <----|---->  RSPI0_SSL----|P5_2  |
        |---------------------------|---------------------------|

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

