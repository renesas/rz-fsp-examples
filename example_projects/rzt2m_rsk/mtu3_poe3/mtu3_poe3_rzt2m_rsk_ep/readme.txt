/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates how to generate three-phase complementary PWM (Pulse Width Modulation) signals
    using the Multi-function Timer Pulse Unit 3 (MTU3) and MTU4 on the RZ/T2M microcontroller. The program utilizes
    the MTU3 Three-Phase to configure and control the PWM outputs. The Port Output Enable 3 (POE3) module is use to
    disable all PWM outputs, effectively stopping the program.

    The main goal is to output six PWM signals (3 positive-phase and 3 negative-phase) with a carrier cycle of 100μs
    and a dead-time of 2 μs between complementary pairs. The duty cycle of the PWM signals can be dynamically changed
    to 25%, 50%, or 75% by press SW2. Additionally, SW1 triggers POE3 to disable all PWM outputs.
    Any API/event failure message is displayed via RTT Viewer.

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

        4. *Destination Selection JA2-A.15:
        SW5.5  : OFF
        SW5.6  : OFF
        SW5.7  : ON

        5. *Destination Selection JA2-A.13:
        SW5.8  : OFF
        SW5.9  : OFF
        SW5.10 : ON

        6. *Destination Selection JA2-A.16:
        SW6.5  : ON
        SW6.6  : OFF

        7. *Destination Selection JA2-A.18:
        SW6.7  : ON
        SW6.8  : OFF

        8. PWM Output

        |  JA2-A header 26 way  |        PWM Output        |
        |-----------------------|--------------------------|
        |    JA2-A-13       ----|---->   Phase U+ (M1_UP)  |
        |    JA2-A-14       ----|---->   Phase U− (M1_UN)  |
        |    JA2-A-15       ----|---->   Phase V+ (M1_VP)  |
        |    JA2-A-16       ----|---->   Phase V− (M1_VN)  |
        |    JA2-A-17       ----|---->   Phase W+ (M1_WP)  |
        |    JA2-A-18       ----|---->   Phase W− (M1_WN)  |
        |-----------------------|--------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
 
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
