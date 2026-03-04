/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates basic functionalities of sdhi driver with FreeRTOS+FAT file system on Renesas
    RZ/A3UL MPU based on Renesas FSP. FreeRTOS+FAT uses the underlying Block media driver.
    The Block media driver utilizes SDHI driver to perform file operations on the SD Card.
    Error and info messages will be printed on Terminal Emulator.

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
    1 x Micro SD Card

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
        |               | SW1-2   : ON (To use eMMC or SD)   |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Default                  |
        |               | SW2     : Default                  |
        |               | SW3     : Default                  |
        |               | SW4     : Default                  |
        |               | SW5     : Default                  |
        |               | SW6     : Default                  |
        |               | SW7     : Default                  |
        |               | SW8     : Default                  |
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
    (4) Insert micro sd card into card slot(CN3) on the Module board.

Note:
0. SD channel 0 is supported.
1. Format the SD Card as FAT32 before performing any operation.
2. Data written can also be manually verified in file rza_sdhi.txt.
3. User is expected to execute "safely remove SD Card" option before removing SD Card else further file operations may fail and
   SD Card data may get corrupted.
4. User has to remove and re-insert the SD Card after executing the "safely remove SD Card" & "Initialize FreeRTOS+FAT" option.
5. If SD Card is removed without "safely remove SD Card" option, user is recommended to perform "safely remove SD Card" and
   "Initialize FreeRTOS+FAT" commands in sequence. This will make sure no operations fails after unexpected removal of SD Card.
6. Operation is not guaranteed for any user input value other than integer, char(i.e. float, special char) through
   Terminal Emulator input.