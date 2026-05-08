/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the use of the Multi-Function Timer Pulse Unit 3 (MTU3) in phase counting mode 1 
    on the RZ/T2H microprocessor. It counts pulses from a two-phase encoder (A phase, B phase) and provides 
    RTT Viewer commands to get, set, and reset the count value. 

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
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x MB057GA140 Encoder (Motor).

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
    (4) Connect the two-phase incremental encoder Phase A/B signals to the GPT using RS-485 conversion boards.

        |      CN24 header      |   MB057GA140(RS-485 Board)        |
        |-----------------------|-----------------------------------|
        |       CN24-PIN9  <----|---->      Phase A                 |
        |       CN24-PIN10 <----|---->      Phase B                 |
        |-----------------------|-----------------------------------|

    (5) Power supply and ground connections for RS‑485 boards:

        |     Evaluation Board  |   MB057GA140(RS-485 Board)        |
        |-----------------------|-----------------------------------|
        |  CN54‑PIN7 (5V)  <----|---->      +5V                     |
        |  CN54‑PIN8 (GND) <----|---->      GND                     |
        |-----------------------|-----------------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000
 
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).

3. In SEGGER J-Link RTT Viewer, set Input->End of Line to 'None'.

4. Encoder wiring via RS-485 conversion boards (MB057GA140):
   Phase A = GREEN wire: connect to RS-485 conversion board #1 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN24-PIN9.
   Phase B = YELLOW wire: connect to RS-485 conversion board #2 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN24-PIN10.

5. All RS‑485 conversion boards share a common 5V and GND supply.
   Power is sourced from the Evaluation Board via CN54 and distributed
   among the RS‑485 boards to ensure a common VCC and GND reference.