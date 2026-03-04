/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates the use of the General Purpose PWM Timer (GPT) in phase counting mode 1 on the 
    RZ/T2M microcontroller. It counts pulses from a two-phase encoder (A phase, B phase) and provides RTT Viewer-based 
    commands to get, set, and reset the count value. 

    On user input via RTT Viewer:
        "g": Retrieves the current count value from the GPT counter.
        "s": Allows the user to set a specific count value. After entering s, the user inputs the desired value.
        "r": Resets the GPT counter value to zero.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x MB057GA140 Encoder(Motor).

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
        SW4.4 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. *Destination Selection JA2-A.14:
        SW5.3 : OFF
        SW5.4 : ON

        4. *Destination Selection JA2-A.10:
        SW5.8  : OFF
        SW5.9  : ON
        SW5.10 : OFF

        5. *Set SW6 following.
        SW6 : OFF

        6. Connect Two-phase encoder Phase A/B signal with GPT

        |  JA2-A header 26 way  |        MB057GA140        |
        |-----------------------|--------------------------|
        |    JA2-A-10      <----|---->     Phase A         |
        |    JA2-A-14      <----|---->     Phase B         |
        |-----------------------|--------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
 
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).

3. Encoder wiring via RS-485 conversion boards (MB057GA140):
   Phase A = GREEN wire: connect to RS-485 conversion board #1 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to JA2-A-10.
   Phase B = YELLOW wire: connect to RS-485 conversion board #2 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to JA2-A-14.