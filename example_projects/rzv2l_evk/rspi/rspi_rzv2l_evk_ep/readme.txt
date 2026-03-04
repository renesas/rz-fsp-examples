/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RSPI HAL module APIs. The project configures RSPI channels
    (Channel 0 and Channel 1) in Master and Slave mode. Once the module is initialized and the channels are configured,
    Master and Slave can transmit and receive data based on commands from the user sent through J-Link RTT Viewer. This
    project supports the DMAC feature. In cases where this feature is not used in the application, please invalid the
    DMAC feature by the following steps.
    - Set DMAC Support to Disabled on the main stack of the driver in FSP Configuration.
    - Remove the DMAC sub stacks that linked to the main stack.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2L-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    4 x Jumper wire.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).

    (2) Set for DIP switches and jumpers as follow:

        +================+================================+
        | Board setting  | RZ/V2L Evaluation Kit (SMARC)  |
        +================+================================+
        | Module board   | SW1-1  : OFF                   |
        |                | SW1-2  : Don't care            |
        +----------------+--------------------------------+
        | Carrier board  | SW1    : Don't care            |
        |                | SW2    : Don't care            |
        |                | SW3    : Don't care            |
        |                | SW4    : Don't care            |
        |                | SW5    : Don't care            |
        |                | SW6    : Don't care            |
        |                | SW7    : Don't care            |
        |                | SW8    : Don't care            |
        |                | SW11-1 : OFF                   |
        |                | SW11-2 : OFF                   |
        |                | SW11-3 : OFF                   |
        |                | SW11-4 : ON                    |
        |                |                                |
        |                | CN4    : Jumper connects 1-3   |
        |                |          Jumper connects 2-4   |
        +----------------+--------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect pins with jumper for SPI communication as follows:

        PMOD1 (SMARC Carrier Board)     PMOD0 (SMARC Carrier Board)
        ------------------              ------------------
        |6  5  4  3  2  1|              |6  5  4  3  2  1|
        |12 11 10 9  8  7|              |12 11 10 9  8  7|
        ------------------              ------------------

        RZ/V2L Evaluation Kit (SMARC):
        PIN         Slave (RSPI1)       Master (RSPI0)
        MISO  ----> P44_2(PMOD0 PIN3) - P43_2(PMOD0 PIN8)
        MOSI  ----> P44_1(PMOD0 PIN2) - P43_1(PMOD0 PIN10)
        CK    ----> P44_0(PMOD0 PIN4) - P43_0(PMOD0 PIN7)
        SSL   ----> P44_3(PMOD0 PIN1) - GND  (PMOD0 PIN11)

Note:
1. User is expected to enter data of size not exceeding 64 bytes.
   As RSPI bit width is set to 32 bits, each 4 bytes of user data input will be transferred in a single RSPI 
   transmission. The bytes sent for master should be bigger than the bytes sent for slave when using WriteRead 
   operation. Operation is not guaranteed for any user input value other than integer, char (i.e., float, special 
   char) through RTT.

2. In the case, the input is single character. Make sure the input of RTT-Viewer is not "None".

3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x601106b8

4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).