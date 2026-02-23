/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates how to use the UART module to send and receive messages via the terminal on the RZ/T2L 
    micro controller. The project initializes the UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate 
    value and transmit a 'Hello World' message to the terminal. It then enters a receive loop, waiting for user input.
    Any data received from the terminal is immediately echoed back.

    Any API or event failure message is displayed via RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    Terminal Console Application: Tera Term or a similar application

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

        3. *This sample program uses Serial USB
        Therefore, the following settings are required.
        SW4.7  : OFF
        CN32   : 1-2 short
        CN34   : 1-2 short
        CN35   : 1-2 short

        4. *USB connection between RSK board and PC
        CN16 - USB mini-B

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
