/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The project initializes GPT module in Periodic, PWM or One-shot mode based on user input from the displayed menu
    options. In periodic mode, user can enter the time period within the permitted ranges to change the blink frequency.
    LD0 and LD3 toggle alternately at the configured period. In PWM mode, user can enter the duty cycle within the
    specified range to control the PWM output. LD0 is driven with PWM at the configured period and duty cycle.
    In One-shot mode, the timer runs once and then expires. On timer expiry, LD0 turns OFF then ON immediately, while
    LD3 turns ON then OFF immediately (opposite states).
    Status and user prompts are displayed on the J-Link RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    FSP Smart Configurator: Version 2026-07
    IAR: Version 9.60.3
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 9.44

3. Hardware Requirements:
    RZ supported boards: RZ/T2N-EVK.
    1 x Renesas RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module (e.g: https://digilent.com/reference/pmod/pmodled/start).
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).

    (2) Set for DIP switches and jumpers as follow.
        Hardware Connection:
        - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
            SW8.1 : ON
            SW8.2 : OFF
            SW8.3 : ON

        - 2nd Boot CPU : Cortex-R52 CPU0
            SW10.10 : ON

        - Use GPT3 (GTIOC3_3A -> Pin 16_6 and GTIOC3_3B -> Pin 16_7)
            SW9.8   : ON
            SW7.3   : OFF

5. Hardware Connections:
    (1) Connect the J7 connector on each evaluation board to a 15V3A power source using a USB Type-C cable for board power supply.
    (2) Connect the J32 port on evaluation board to the PC using micro USB cable for debugging.
    (3) Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.
    (4) Connect PMOD1 (J34) pins 1–6 on board to the PMOD LED for LED output control. 

Note:
1. Timer period range in millisecond as per example project configuration:
   RZ/T2N      --      (0 to 10737) ms
2. User is expected to enter data of size not exceeding above 15 bytes.
   Operation is not guaranteed for any user input value other than integer, char (i.e. float, special char)
   through RTT.
3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000
   b. iar: 0x10180000
4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
