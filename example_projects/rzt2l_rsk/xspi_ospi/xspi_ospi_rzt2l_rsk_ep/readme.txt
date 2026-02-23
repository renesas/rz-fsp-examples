/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates typical use of the xSPI_OSPI driver with an external OctaFlash in OPI-DTR mode.
    The program temporarily sets 1S-1S-1S, enables OPI-DTR via CR2, then switches the controller to 8D-8D-8D.
    A test sector is erased; Page Program writes data, a high-speed read verifies it, and the same location
    is checked again via the non-cacheable memory-mapped mirror.
    Finally, write a small buffer and confirm via mapped reads; status is printed to the RTT Viewer.

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
        16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.4 : OFF.

        3. *The xSPI0 unit require 1.8V supply to drive OctaFlash.
        Therefore, the following settings (VCC1833_3 = 1.8V) are required.
        SW4.7 : ON
        CN32  : 2-3 short

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).

3. ChipErase is a time-consuming process, so it is commented out when the program is provided.
   How to use Chip Erase:
   (1). Comment out the "Sector Erase" code block in hal_entry() function.
   (2). Uncomment the "Chip Erase" code block in hal_entry() function.
   (3). Re-build the project
   (4). Download EP to RZ board and run the project. Then confirm Chip Erase operation via the RTT Viewer.
