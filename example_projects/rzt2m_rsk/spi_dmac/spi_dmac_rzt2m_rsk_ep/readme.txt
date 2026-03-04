/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates a typical use case of the SPI module on the RZ/T2M microcontroller. The project 
    initializes SPI driver and configures 2 SPI channels in Master (Channel 0) and Slave mode (Channel 2) on the
    RZ/T2M microcontroller. After initialization, the master and slave can transmit and receive data based on commands
    from user sent through JLinkRTTViewer.

    On user input via RTT Viewer:
        1: Demonstrate SPI Write and Read individually.
        2: Demonstrate SPI Write Read simultaneously.
        3: Close all opened SPI modules and exits.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please disable the DMAC feature by the following steps.
    - Set DMAC on Transmission and Reception to Disabled on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub-stacks that are linked to the main stack.

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
        SW4.4 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. *Set SW6 following:
        SW6.1 : ON

        8. Connect SPI ch0 and ch2 pins as follows

        |  JA3-A header 50 way  |        J26 Grove        |
        |     SPI ch0 Master    |      SPI ch0 Slave      |
        |-----------------------|-------------------------|
        |    JA3-A-20       ----|---->   J26.1 (SSL)      |
        |    JA3-A-44       ----|---->   J26.2 (MOSI)     |
        |    JA3-A-21       ----|---->   J26.3 (MISO)     |
        |    JA3-A-25       ----|---->   J26.4 (SCK)      |

Note:
1. User is expected to enter data of size not exceeding 64 bytes.
   As SPI bit width is set to 32 bits, each 4 bytes of user data input will be transferred in a single SPI transmission.
   The bytes sent for master should be bigger than the bytes sent for slave when using WriteRead operation.
   Operation is not guaranteed for any user input value other than integer, char (i.e., float, special char) through RTT.

2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
 
3. If an EP is modified, compiled and downloaded, please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).

4. In this EP, the TRACE debugging feature has been disabled to allow SPI Channel 2 usage. Developers can still perform 
   debugging through standard JTAG or SWD interfaces, which remain fully supported.