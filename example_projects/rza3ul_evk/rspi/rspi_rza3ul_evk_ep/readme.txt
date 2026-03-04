/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RSPI HAL module APIs.
    The project configure RSPI channels (Channel 1 and Channel 0) in Master and Slave mode.
    Once the module is initialised and the channels are configured,
    Master and Slave can transmit and receive data based on commands from user sent through Terminal Emulator.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disabled on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack .

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.

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
        |               | SW1-2   : Default                  |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Default                  |
        |               | SW2     : Default                  |
        |               | SW3     : Default                  |
        |               | SW4     : Default                  |
        |               | SW5     : 1-2                      |
        |               | SW6     : 1-2                      |
        |               | SW7     : 1-2                      |
        |               | SW8     : 1-2                      |
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
    (4) Connect pins with jumper as follows.

        PMOD1 (SMARC Carrier Board)     PMOD0 (SMARC Carrier Board)
        ------------------              ------------------
        |6  5  4  3  2  1|              |6  5  4  3  2  1|
        |12 11 10 9  8  7|              |12 11 10 9  8  7|
        ------------------              ------------------

        +----------+---------------------------+---------------------------+
        | SPI Pin  |       Master board        |        Slave board        |
        +----------+---------------------------+---------------------------+
        |  MISO    |    P4_2 (PMOD0 Pin3)  <-- | -->  P2_3 (PMOD1 Pin8)    |
        |  MOSI    |    P4_1 (PMOD0 Pin2)  <-- | -->  P2_2 (PMOD1 Pin7)    |
        |  CK      |    P4_0 (PMOD0 Pin4)  <-- | -->  P1_0 (PMOD0 Pin8)    |
        |  SSL     |    P4_3 (PMOD0 Pin1)  <-- | -->  P1_3 (PMOD0 Pin9)    |
        +----------+---------------------------+---------------------------+

Note:
    User is expected to enter data of size not exceeding 64 bytes. As RSPI bit width is set to 32 bits,
    each 4 bytes of user data input will be transferred in single RSPI transmission.
    The bytes sent for master should be bigger than the bytes sent for slave when using WriteRead operation.
    Operation is not guaranteed for any user input value other than integer, char(i.e. float, special char)
    through Terminal Emulator.