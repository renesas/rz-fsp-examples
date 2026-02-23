/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates the basic functionalities of LCDC device driver on Renesas RZ/A3M MPUs based on
    Renesas FSP. On successful initialization of LCDC module, the EP will be able to read the display resolution and
    frame buffer set in the LCDC module and draw color bands to cover the entire screen of external LCD connected to
    RZA MPUs. API Failure messages will be displayed on Terminal Emulator.

2. Software Requirement:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x MIPI Graphics Expansion Board 2 Version 1 (APP_LCD_EK_MIPI_2).

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
    (1) Connect MIPI Graphics Expansion Board 2 Version 1 (APP_LCD_EK_MIPI_2) to J32 on Board to use the LCD.
    (2) Connect the DEBUG1 port of the RZ board to the PC using a USB Type-C cable for board power supply, debugging,
    and serial terminal communication (Tera Term).

Note:
    User can also view the raw image in memory monitor of the e2studio IDE using the address of g_framebuffer and
    specifying the resolution.
