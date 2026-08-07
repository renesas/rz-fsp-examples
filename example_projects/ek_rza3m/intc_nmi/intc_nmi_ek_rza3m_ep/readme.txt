/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the INTC NMI module APIs.
    The project initializes the NMI interrupt in Interrupt Controller.
    User is requested to press the button on PMOD BUTTON (BTN0) to trigger the external NMI 
    and this then will start toggling of user LED on-board (LED1).

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD BUTTON.
    3 x jumper wire.

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
    (1) Connect the DEBUG1 port (J10) of the RZ board to the PC using a USB Type-C cable for board power supply, debugging, and
    serial terminal communication (Tera Term).

    (2) Connect J1 on PMOD BUTTON to J2 on EK-RZ/A3M board with jump wire as follows.
        |-----------------------|--------------------------|
        | J1 (PMOD BUTTON)      |   J2 (EK-RZ/A3M Board)   |
        |-----------------------|--------------------------|
        |  1 --- BTN0      <----|---->  NMI_L              |
        |  2 --- BTN1           |                          |
        |  3 --- BTN2           |                          |
        |  4 --- BTN3           |                          |
        |  5 --- GND       <----|---->  GND                |
        |  6 --- VCC       <----|---->  +3.3V              |
        |-----------------------|--------------------------|
Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none