/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
     The example project demonstrates the typical use of the SPI HAL module APIs. The project configures 
     SPI channels (Channel 3 and Channel 2) in Master and Slave mode. Once the module is initialized and the 
     channels are configured, Master and Slave can transmit and receive data based on commands from the user sent through 
     JLinkRTTViewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/N2L-RSK.
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

        2. *Settings as below.
        SW4.6   : OFF
        SW4.7   : ON
        SW8.8   : OFF
        SW8.9   : OFF
        SW8.10  : ON
        SW11.8  : OFF
        SW11.9  : OFF
        SW11.10 : ON
        CN25    : 1-2 short

        3. *Connect SPI ch2 and ch3 pins following.
        Pins   : SPI ch2    SPI ch3
        ---------------------------
        SSL    : J26.1 - CN1.6
        MOSI   : J26.2 - CN1.4
        MISO   : J26.3 - CN1.3
        SCK    : J26.4 - CN1.2

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000.

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).