/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates typical use of the xSPI_QSPI driver with an external Serial NOR flash.
    Configure xSPI for QSPI operation, enable Quad mode and 4-byte addressing, then erase a test sector.
    Issue Page Program (0x02) and Fast Read (0xEB) at a test address and verify the data.
    Read the same location via the memory-mapped region (direct read) and verify again.
    Finally, write a small buffer and confirm via mapped reads; status is printed to the RTT Viewer.

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

        2. *When using QSPI(IC21) by connecting xSPI, 
        short the jumper CN8-Pin2-3.
        short the jumper CN24-Pin2-3.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).

3. ChipErase is a time-consuming process, so it is commented out when the program is provided.
   How to use Chip Erase:
   (1). Comment out the "Sector Erase" code block in hal_entry() function.
   (2). Uncomment the "Chip Erase" code block in hal_entry() function.
   (3). Re-build the project
   (4). Download EP to RZ board and run the project. Then confirm Chip Erase operation via the RTT Viewer.