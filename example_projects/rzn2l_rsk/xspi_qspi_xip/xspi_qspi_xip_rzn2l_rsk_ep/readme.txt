/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates execution from external Serial Flash memory using xSPI in XiP mode.
    Initialize the xSPI QSPI interface, then enable Quad mode and enter XiP.
    Run the blinky LED routine while code is fetched from the Serial Flash.
    Exit XiP and disable Quad mode to return to standard SPI operation.
    Progress and results are printed to the RTT Viewer.

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
        xSPI0 boot mode (x1 boot Serial flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. * Set switch in xSPI0 boot mode.
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON

       2. *To use LED, set SW to:
        SW 4.6 : OFF
        SW 4.7 : ON
     
        SW 8.1 : OFF
        SW 8.2 : ON
        SW 8.3 : OFF
     
        SW11.1 : ON
        SW11.2 : OFF
        SW11.3 : OFF
     
       3. *Use xSPI0 unit
        CN8    : 2-3 short
        CN24   : 2-3 short

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).

3. Before powering on the board, set the boot mode for RAM execution without external flash, then flash the Hex
   file to allow safe flash programming without interference from existing code. After flashing, switch the boot
   mode back to xSPI0, power-cycle the board and attach SEGGER RTT Viewer to the _SEGGER_RTT address.