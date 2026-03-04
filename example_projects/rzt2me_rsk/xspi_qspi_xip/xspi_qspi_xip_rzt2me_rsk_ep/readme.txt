/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates execution from external Serial Flash memory using xSPI in XiP mode. Initialize
    the xSPI QSPI interface, then enable Quad mode and enter XiP. Run the blinky LED routine while code is fetched from
    the Serial Flash. Exit XiP and disable Quad mode to return to standard SPI operation. Progress and results are
    printed to the RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2ME-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-microB cable(when using the J-Link on board).

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI0 boot mode (x1 boot Serial flash).

    (2) Set for DIP switches and jumper as follow:
        +===============+==========================+
        | Board setting | RZ/T2ME-RSK              |
        +===============+==========================+
        | Module board  | SW4-1  : ON              |
        |               | SW4-2  : ON              |
        |               | SW4-3  : ON              |
        |               | SW4-4  : Default         |
        |               | SW4-5  : OFF             |
        +---------------+--------------------------+

        - The user needs to short the jumper CN8 Pin2-3 to use QSPI(IC21).
        - The user needs to open the jumper J9 when using the J-Link OB.

5. Hardware Connections:
    (1) Connect the CN5 port of the RZ board to a 5V power source using a USB Type-C cable for board power supply.
    (2) Connect the J10 port of the RZ board to the PC using a USB Type-microB cable for board debugging when using the
        J-Link OB.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
3. Before powering on the board, set the boot mode for RAM execution without external flash, then flash the Hex
   file to allow safe flash programming without interference from existing code. After flashing, switch the boot
   mode back to xSPI0, power-cycle the board and attach SEGGER RTT Viewer to the _SEGGER_RTT address.