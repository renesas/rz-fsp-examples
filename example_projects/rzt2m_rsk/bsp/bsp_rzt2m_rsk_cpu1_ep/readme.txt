/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates basic functionalities of BSP dual cores of CPU0 and CPU1.
    After the power is turned on, the program on the CPU0 side makes initial settings,
    copies the program code for CPU1 from the external flash memory to the RAM (the RAM execution 
    without flash memory version does not copy). Then, the program releases the reset of CPU1.
    The program on the CPU1 side performs initial settings after software reset is released, 
    and toggles on-board LED2 and LED3.

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
        SW4.5 : OFF

        3. *Set SW6 as following:
        SW6.3 : ON

        4. *Set jumper setting:
        CN8  : jumper 2-3 short
        CN17 : jumper 2-3 short
        J8   : jumper 1-2 short

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x301808c0

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
