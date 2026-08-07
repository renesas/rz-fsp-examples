/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the erase and write functionalities of the SPI flash device on RZ/A3M MPU based
    on Renesas FSP.
    The sample code is executed by selecting the menu number on the console.
    In main menu, user selects operation for the flash device.
        1. Erase and verify.
        2. Write random data and verify.
    And then, user inputs the target address and size.
    After, the sample code execute the operation and show the result.
    LED1 blinks while running the project.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
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
    (1) Connect the DEBUG1 port (J10) of the RZ board to the PC using a USB Type-C cable for board power supply, debugging, and
    serial terminal communication (Tera Term).

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

2. User is expected to enter the erase start address and the write start address equal to or greater than 0x40000 to
   prevent example code from being corrupted by itself.
   The erase start address must be aligned with one of the "Erase Size" in FSP configuration: 4096, 32768, 65536.
   The erase size must be multiple of the "Erase Size" matched by the previous item.
   The write size must be greater than 1.