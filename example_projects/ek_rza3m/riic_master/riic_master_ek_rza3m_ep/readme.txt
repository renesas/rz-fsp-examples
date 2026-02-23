/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RIIC master HAL module APIs. The project initializes RIIC
    master module with fast mode and interfaces with PmodACL™ Board for ADXL345. On power up after establishing the
    connection of sensor with RZ/A3M board, it displays accelerometer axis data on Terminal Emulator. Any API/event
    failure will be displayed on Terminal Emulator. This project supports the DMAC feature. In cases where this feature
    is not used in the application, please invalid the DMAC feature by the following steps: set DMAC Support to
    Disabled on the main stack of the driver in FSP Configuration and then remove the DMAC sub stacks that linked to
    the main stack.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod ACL.

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
    (1) Connect Pmod ACL and Board as follows.
        Pmod ACL has two on board connectors, J2 is used for I2C communication.
        +----------+-----------------+-----------------+
        | I2C Pin  |     Board       |    Pmod ACL     |
        +----------+-----------------+-----------------+
        |   SDA    |    J1 Pin2   <--|-->  J2 Pin2     |
        |   SCL    |    J1 Pin4   <--|-->  J2 Pin1     |
        |   GND    |    J1 Pin39  <--|-->  J2 Pin3     |
        |   VCC    |    J1 Pin5   <--|-->  J2 Pin4     |
        +----------+-----------------+-----------------+

    (2) Connect the DEBUG1 port of the RZ board to the PC using a USB Type-C cable for board power supply, debugging,
        and serial terminal communication (Tera Term).
