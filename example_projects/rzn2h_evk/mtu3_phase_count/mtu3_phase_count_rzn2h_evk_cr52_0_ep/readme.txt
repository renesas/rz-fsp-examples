/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the use of the Multi-Function Timer Pulse Unit 3 (MTU3) in phase counting mode 1 
    on the RZ/N2H microprocessor. It counts pulses from a two-phase encoder (A phase, B phase) and provides 
    RTT Viewer-based commands to get, set, and reset the count value. 

    On user input via RTT Viewer:
        "g": Retrieves the current count value from the MTU3 counter.
        "s": Allows the user to set a specific count value. After entering s, the user inputs the desired value.
        "r": Resets the MTU3 counter value to zero.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x MB057GA140 Encoder (Motor).

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        - The operating mode of the RZ/N2H : xSPI1 boot mode (x1 boot serial flash)
          DSW3.1   : ON
          DSW3.2   : OFF
          DSW3.3   : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4   : OFF

        - Use CN43-PIN25 and CN43-PIN27 as MTU3 (MTCLK0A/P03_3 and MTCLK0B/P03_4)
          DSW7.1   : OFF
          DSW7.2   : ON
          DSW7.3   : OFF
          DSW7.4   : ON

5. Hardware Connections:
    (1) Connect the CN26 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN13 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (3) Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
    (4) Connect Two-phase encoder Phase A/B signal with MTU3

    |      CN43 header      |        MB057GA140        |
    |-----------------------|--------------------------|
    |       CN43-PIN25 <----|---->     Phase A         |
    |       CN43-PIN27 <----|---->     Phase B         |
    |-----------------------|--------------------------|
    (5) Power supply and ground connections for RS‑485 boards:

    |     Evaluation Board  |   MB057GA140(RS-485 Board)        |
    |-----------------------|-----------------------------------|
    |  CN34‑PIN7 (5V)  <----|---->      +5V                     |
    |  CN34‑PIN8 (GND) <----|---->      GND                     |
    |-----------------------|-----------------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called 
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).

3. Encoder wiring via RS-485 conversion boards (MB057GA140):
   Phase A = GREEN wire: connect to RS-485 conversion board #1 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN43-PIN25.
   Phase B = YELLOW wire: connect to RS-485 conversion board #2 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN43-PIN27.

4. In SEGGER J-Link RTT Viewer, set Input->End of Line to 'None'.

5. All RS‑485 conversion boards share a common 5V and GND supply.
   Power is sourced from the Evaluation Board via CN34 and distributed
   among the RS‑485 boards to ensure a common VCC and GND reference.