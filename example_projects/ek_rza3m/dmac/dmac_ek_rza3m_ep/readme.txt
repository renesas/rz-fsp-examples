/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The project initializes 2 DMAC transfer instances. Upon successful initialization, a menu option is displayed.
    On selecting 1 from menu option, GTM generates 100ms interrupt to trigger a transfer from the source buffer
    to port control register for 60 times indicated by LED toggling for 60 times. On selecting 2 from menu option,
    DMAC (runs in Block mode) transfers data from the MTU counter register to destination which gets printed to
    Terminal Emulator.

2. Software Requirements:
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
