/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RIIC master HAL module APIs. The project initializes RIIC
    master module with fast mode and interfaces with PmodACL Board for ADXL345. On power up after establishing the
    connection of sensor with the board, it displays accelerometer axis data on RTT Viewer. Any API/event failure will
    be displayed on RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2L-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD ACL module.
    4 x Jumper wire.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).
    (2) Set for DIP switches and jumpers as follow.

        +===============+===============================+
        | Board setting | RZ/V2L Evaluation Kit (SMARC) |
        +===============+===============================+
        | Module board  | SW1-1  : OFF                  |
        |               | SW1-2  : Don't care           |
        +---------------+-------------------------------+
        | Carrier board | SW1    : Don't care           |
        |               | SW2    : Don't care           |
        |               | SW3    : ON                   |
        |               | SW4    : ON                   |
        |               | SW5    : Don't care           |
        |               | SW6    : Don't care           |
        |               | SW7    : Don't care           |
        |               | SW8    : Don't care           |
        |               | SW11-1 : OFF                  |
        |               | SW11-2 : OFF                  |
        |               | SW11-3 : OFF                  |
        |               | SW11-4 : ON                   |
        |               |                               |
        |               | CN4    : Jumper connects 1-3  |
        |               |          Jumper connects 2-4  |
        +---------------+-------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect Pmod ACL and SMARC Carrier Board as follows.
        Pmod ACL has two on board connectors, J2 is used for I2C communication.

        J2 (Pmod ACL)                  PMOD1 (SMARC Carrier Board)
                                       -----
                                       |1  |
        -----                          |2  |
        |5  |---SCL  --->  RIICx_SCL---|3  |
        |6  |---SDA  --->  RIICx_SDA---|4  |
        |7  |---GND  --->  GND pin  ---|5  |
        |8  |---VCC  --->  VCC pin  ---|6  |
        -----                          -----

Note:
1. Segger RTT block address may be needed to download and observe example project operation
   using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x60110628

2. If an example project is modified, compiled, and downloaded please find the block address
   (for the variable in RAM called _SEGGER_RTT) in .map file generated in the build
   configuration folder (Debug/Release).