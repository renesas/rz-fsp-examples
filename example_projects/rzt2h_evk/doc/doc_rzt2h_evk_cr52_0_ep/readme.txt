/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the usage of the DOC HAL module on RZT2H devices.
    The project showcases DOC event-based operations configured via the FSP configurator.
    Each DOC event is selected and verified individually, including Comparison Match, Comparison Mismatch,
    Addition Overflow, and Subtraction Underflow.
    On RZT2H, comparison events are verified by polling the DOC operation complete flag,
    while arithmetic error events are verified using the PERI_ERR0 interrupt mechanism.
    The operation status and verification results are displayed on the RTT Viewer.
    Any API error or unexpected DOC behavior will be reported on the console.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 9.44

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        - The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
          SW14.1 : ON
          SW14.2 : OFF
          SW14.3 : ON
          SW14.6 : OFF

        - CPU0 ATCM wait cycle = 1 wait cycle
          SW14.4 : OFF

5. Hardware Connections:
    (1) Connect the CN14 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN46 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (3) Slide SW16 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).

3. This example verifies one DOC event per build. By default, the DOC event is set to Comparison Mismatch.
   To test a different DOC event, update the Event selection in the DOC module settings
   via the FSP Configurator (Module -> Event), then rebuild and run the project.
   Each event is validated independently according to its expected behavior on RZT2H devices.