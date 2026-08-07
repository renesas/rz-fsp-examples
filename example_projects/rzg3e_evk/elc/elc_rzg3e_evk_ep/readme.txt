/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ELC HAL module APIs.
    ELC Software Event, GPT0 and GPT1 events are linked using ELC. The start source for GPT0 and GPT1 is ELC
    Software Event and the stop source for GPT0 is GPT1 counter overflow. GPT0 runs in PWM mode and GPT1 runs
    in One-shot mode. On giving valid RTT input, an ELC Software Event is generated that triggers LED blinking.
    LED stops blinking after 5 sec when GPT0 expires.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 9.44
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod LED.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+============================================+
        | Board setting | RZ/G3E Evaluation Kit (SMARC)              |
        +===============+============================================+
        | Module board  | SW5-1  : OFF                               |
        |               | SW5-2  : OFF                               |
        |               | SW5-3  : OFF                               |
        |               | SW5-4  : OFF                               |
        |               | SW5-5  : OFF                               |
        |               | SW5-6  : OFF                               |
        +---------------+--------------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                     |
        |               | SW_MODE-2        : ON                      |
        |               | SW_MODE-3        : OFF                     |
        |               | SW_MODE-4        : ON                      |
        |               | SW_GPIO_CAN_PMOD : 3-2                     |
        |               | PMOD_PWR_SEL     : Jumper connects 1-3     |
        |               |                  : Jumper connects 2-4     |
        +---------------+--------------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C_PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for board 
        power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD0_2A (PIN7-12) on SMARC Carrier Board and Pmod LED (Blink on LED LD0).

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08007428

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
