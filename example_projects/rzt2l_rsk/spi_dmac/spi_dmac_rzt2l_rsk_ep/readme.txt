/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates a typical use case of the SPI module on the RZ/T2L microcontroller. The project 
    initializes SPI driver and configures 2 SPI channels in Master (Channel 1) and Slave mode (Channel 3).
    After initialization, the master and slave can transmit and receive data based on commands
    from user sent through JLinkRTTViewer.

    On user input via RTT Viewer:
        1: Demonstrate SPI Write and Read individually.
        2: Demonstrate SPI Write Read simultaneously.
        3: Close all opened SPI modules and exits.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2L-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI0 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *SW4 settings must be below to start this sample program.
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.4 : OFF

        3. *This sample program uses SPI1
        Therefore, the following settings are required.
        SW4.6  : OFF
        SW8.8  : OFF
        CN17   : 1-2 short

        4. *This sample program uses SPI3
        Therefore, the following settings are required.
        SW5.1 : OFF
        SW5.2 : ON
        SW5.3 : OFF
        SW5.4 : OFF
        SW5.5 : OFF
        SW5.6 : OFF
        SW5.7 : ON
        SW5.8 : OFF
        SW5.9 : OFF
        SW6.1 : OFF
        SW6.2 : ON
        SW6.3 : OFF
        SW6.4 : OFF
        SW6.5 : OFF
        SW6.6 : ON
        SW6.7 : OFF
        SW6.8 : OFF
        E2 : Open
        E3 : Open
        E23 : Short
        E24 : Short

        5. *Connect SPI ch1(master) and ch3(slave) pins following.
        Pins : SPI ch1  - SPI ch3
        ------------------------------------
        SCL  : JA3-A.6  - J26.1
        MOSI : JA3-A.8  - J26.2
        MISO : JA3-A.7  - J26.3
        SCK  : JA3-A.48 - J26.4
        ------------------------------------

Note:
1. User is expected to enter data of size not exceeding 64 bytes.
   As SPI bit width is set to 32 bits, each 4 bytes of user data input will be transferred in single SPI transmission.
   The bytes sent for slave should be lesser than the bytes sent for master when using WriteRead operation.
   Operation is not guaranteed for any user input value other than integer, char(i.e. float, special char) through RTT-Viewer.

2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000.

3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).