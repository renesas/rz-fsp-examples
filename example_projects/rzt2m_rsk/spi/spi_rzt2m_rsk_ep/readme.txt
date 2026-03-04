/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
     The example project demonstrates the typical use of the SPI HAL module APIs. The project configures 
     SPI channels (Channel 0 and Channel 2) in Master and Slave mode. Once the module is initialized and the 
     channels are configured, Master and Slave can transmit and receive data based on commands from the user sent through 
     JLinkRTTViewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. *Set SW6 following.
        SW6.1 : ON

        4. *Connect SPI ch0 and ch2 pins following.
        Pins   : SPI ch0    SPI ch2
        ---------------------------
        SSL    : JA3-A.20 - J26.1
        MOSI   : JA3-A.44 - J26.2
        MISO   : JA3-A.21 - J26.3
        SCK    : JA3-A.25 - J26.4

Note:
1. User is expected to enter data of size not exceeding 64 bytes.
   As SPI bit width is set to 32 bits, each 4 bytes of user data input will be transferred in a single SPI transmission.
   The bytes sent for master should be bigger than the bytes sent for slave when using WriteRead operation.
   Operation is not guaranteed for any user input value other than integer, char (i.e., float, special char) through RTT.

2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT)
   in .map file generated in the build configuration folder (Debug/Release).