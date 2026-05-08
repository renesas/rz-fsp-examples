/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates basic functionalities of SDHI driver with FreeRTOS+FAT file system on Renesas
    RZ/A3M MPU based on Renesas FSP. FreeRTOS+FAT uses the underlying Block media driver.
    The Block media driver utilizes SDHI driver to perform file operations on the SD Card.
    Error and info messages will be printed on Terminal Emulator.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Micro SD Card.

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

    (4) The user needs to place jumber J15 on pins 2-3 to use the CD pin.

5. Hardware Connections:
    (1) Connect the DEBUG1 port (J10) of the RZ board to the PC using a USB Type-C cable for board power supply, debugging, and
    serial terminal communication (Tera Term).

    (2) Insert micro SD card into card slot (J11) on the EK-RZ/A3M.

Note:
1. To echo back characters types in Tera Term, the user needs to enable it through
        [Setup]  -> [Terminal...] -> Check [Local echo]

2. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

3. SD channel 0 is supported.

4. Format the SD Card as FAT32 before performing any operation.

5. Data written can also be manually verified in file rza_sdhi.txt.

6. User is expected to execute "safely remove SD Card" option before removing SD Card else further file operations may fail and
   SD Card data may get corrupted.

7. User has to remove and re-insert the SD Card after executing the "safely remove SD Card" & "Initialize FreeRTOS+FAT" option.

8. If SD Card is removed without "safely remove SD Card" option, user is recommended to perform "safely remove SD Card" and
   "Initialize FreeRTOS+FAT" commands in sequence. This will make sure no operations fails after unexpected removal of SD Card.
9. Operation is not guaranteed for any user input value other than integer, char(i.e. float, special char) through
   Terminal Emulator input.