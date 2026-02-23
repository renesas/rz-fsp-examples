/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ELC module APIs.
    Input edge detection of single input port 0 and GPT7 events are linked using ELC.
    The start source for GPT7 is  ELC Single Port 0 Event. GPT7 runs in Periodic mode.
    On giving valid RTT input, a high level from single port triggers timer to start counting.
    Counter value will be displayed on RTT viewer.

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

        3. *This sample program uses P18_1 on J26-7
        Therefore, the following settings are required.
        SW7.7  : ON
        SW7.8  : OFF
        SW7.9  : OFF
        SW7.10 : OFF

        4. *This sample program uses P05_6 on J26_10
        Therefore, the following settings are required.
        SW8.8  : OFF

        5. *Short J26-7 (P18_1) and J26-10 (P05_6) port pins.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).