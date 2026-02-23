/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/
 
1. Project Overview:
    The example project demonstrates the typical use of the UART HAL module APIs. The project initializes the UART with
    Baud rate of 115200 bps and GTM. Using a Terminal Emulator user can provide a value & press enter key to set the
    cycle of the USER LED signal. This project supports the DMAC feature. In cases where this feature is not used in
    the application, please invalid the DMAC feature by the following steps: set DMAC Support to Disabled on the main
    stack of the driver in FSP Configuration and then remove the DMAC sub stacks that linked to the main stack .

2. Software Requirement:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.

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
    Connect the DEBUG1 port of the RZ board to the PC using a USB Type-C cable for board power supply, debugging, and
    serial terminal communication (Tera Term).
