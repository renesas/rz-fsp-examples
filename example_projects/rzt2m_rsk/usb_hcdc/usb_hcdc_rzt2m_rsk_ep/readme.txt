/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates the basic functionality of USB Host Communication Class (HCDC) driver on Renesas RZ MPUs
    using 2 RZ/T2M boards. Board 1 (with USB HCDC Example Project running on it) communicates with Board 2 (with USB PCDC Example Project
    running). Board 1 initiates the communication by sending commands to Board 2 and Board 2 responds by sending the data. Board 1 prints
    the received data on the RTTViewer

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    2 x RZ board.
    2 x USB Type-C cable.
    2 x USB Type-A to micro USB cable.
    1 x USB Type-A to mini USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot Mode :External 16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:
        1. External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. Set SW6 following.
        SW6 : OFF

        3. Connect USB network 
        Connect the USB Type-A port of the board running USB_HCDC to the Mini-USB port of the board running USB_PCDC.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).