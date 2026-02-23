/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates the xSPI HyperBus driver with an external HyperRAM on CS1 of xSPI0.
    At start-up, open the xSPI HyperBus instance, and dumps key register settings for sanity check.
    A memory-mapped test writes/reads pattern data across the HyperRAM range while checking for AHB bus errors and DS timeouts.
    The demo then issues manual-command reads of Device ID Registers and Configuration registers, converting from big-endian before printing.
    The xSPI clock and overall pass/fail status are displayed on the RTT Viewer.

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

        3. *Connect 1.8V Power rail to VCC1833_3.
        SW4.7 : ON
        CN32  : 2-3 Short

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).