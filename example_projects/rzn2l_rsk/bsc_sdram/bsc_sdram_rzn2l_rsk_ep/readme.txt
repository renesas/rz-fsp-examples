/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates basic functionalities of BSC SDRAM driver.
    Project initializes a BSC SDRAM instance. Upon successful initialization, BSC SDRAM executes write data to 
    CS3 mirror area connected to SDRAM and read data after writing. Then, BSC SDRAM executes write 0xFFFF data 
    to CS3 mirror area connected to SDRAM and  read data after writing.
    Any failure will also be displayed on J-Link RTT viewer.

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

        2. *To input the signal to the NOR flash memory, set the RSK board as follows.
        SW4.7 : OFF
        CN17  : 1-2 short
        CN20  : 2-3 short
        CN21  : 2-3 short
        CN22  : 2-3 short
        CN24  : 1-2 short
        CN25  : 1-2 short

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000.

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).