/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates how to use the UART module to send and receive messages via the terminal on the RZ/T2M
    micro controller. The project initializes the UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate 
    value and transmits a 'Hello World' message to the terminal. It then enters a receive loop, waiting for user input.
    Any data received from the terminal is immediately echoed back.

    Any API or event failure message is displayed via RTT Viewer.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please disable the DMAC feature by the following steps.
    - Set DMAC on Transmission and Reception to Disabled on the UART main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub-stacks that are linked to the main stack.
 
2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    Terminal Console Application: Tera Term or a similar application

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

        3. *Use UART data for USB-to-serial conversion
        CN16  : Connect PC to USB Mini-B port 

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
 
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
