/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates a typical use case of the DMAC module. It initializes the DMAC functionality 
    and performs data transfers from a source array located in ATCM to a destination array.
    The transfers are triggered using both a software trigger and a CMTW compare match event.
    Upon successful completion, the received data is displayed in the RTT Viewer.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:
    - The operating mode of the RZ/N2H
        DSW3.1   : ON
        DSW3.2   : OFF
        DSW3.3   : ON

    CPU0 ATCM wait cycle = 1 wait cycle
        DSW3.4   : OFF

    Use LED3
        DSW18.7  : ON
        DSW18.8  : OFF

    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN26 - USB Type-A to micro USB cable.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).