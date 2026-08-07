/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of the RIIC slave HAL module APIs.
    The project initializes RIIC slave and RIIC master module with standard rate
    and is made interfaced with loop-back mechanism. It performs Slave read and 
    write operation continuously once initialization is successful. On successful 
    I2C transaction (6 bytes), Data transceived is compared. LED blinks when data 
    matches; otherwise, it is turned ON as sign of failure. Output message for both 
    corresponding slave operations is displayed on Terminal Emulator. Any API/event
    failure message is also displayed.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disabled on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    2 x jumper wires.

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

    (2) Connect SDA pins and SCL pins between 2 channels as follow:

        |Pin Header J2 (Board)  |     Pin Header J1 (Board)|
        |-----------------------|--------------------------|
        |  Pin 14|---SDA0  <----|---->  SDA1---|Pin 2|     |
        |  Pin 16|---SCL0  <----|---->  SCL1---|Pin 4|     |
        |-----------------------|--------------------------|

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none