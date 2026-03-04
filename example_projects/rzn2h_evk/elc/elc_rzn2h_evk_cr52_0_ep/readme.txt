/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ELC module APIs.
    Input edge detection of single input port 0 and GPT9 events are linked using ELC.
    The start source for GPT9 is  ELC Single Port 0 Event. GPT9 runs in Periodic mode.
    On giving valid RTT input, a high level from single port triggers timer to start counting.
    Counter value will be displayed on RTT viewer.

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
        RAM execution
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

    1. *Configuration of RZ/N2H Evaluation board by Switches and Jumper Blocks
    The operating mode of the RZ/N2H : xSPI1 boot mode (x1 boot serial flash)
        DSW3.1   : ON
        DSW3.2   : OFF
        DSW3.3   : ON

    2. CPU0 ATCM wait cycle = 1 wait cycle
       DSW3.4   : OFF

    3. *Configuring the board circuit
    Connect GPIO and ELC Single Port 0 pins following
        CN46.18 - CN46.13

    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN26 - USB Type-A to micro USB cable.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT)
   in .map file generated in the build configuration folder (Debug/Release).