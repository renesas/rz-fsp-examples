/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The project demonstrates the basic functionalities of POEG driver on Renesas RZG MPUs based on Renesas FSP.
    RZG MPU is used to demonstrate the various operating modes of a POEG driver.
    GPT channels are configured to generate output via the GTIOCxA and GTIOCxB pins.
    The Port Output can be disabled using the below three options
    1. Using POEG trigger pin - This mode supports disabling GPT output pins based on GTETRG input pin level
    2. Using POEG software API - This mode supports disabling GPT output pins by R_POEG_OutputDisable() software API request
    3. Using POEG trigger GPT output level - This mode supports disabling GPT output pin using POEG trigger GPT output level
    4. Reset the POEG status - This option resets the POEG0 and POEG6 status 
    The Port Output can be enabled using the POEG Reset API.
    The status of each of the mode is displayed on J-Link RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module.
    1 x jumper wire.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+========================================+
        | Board setting | RZ/G3E Evaluation Board Kit            |
        +===============+========================================+
        | Module board  | BOOT-1           : OFF                 |
        |               | BOOT-2           : OFF                 |
        |               | BOOT-3           : OFF                 |
        |               | BOOT-4           : OFF                 |
        |               | BOOT-5           : OFF                 |
        |               | BOOT-6           : OFF                 |
        |               | SYS-5            : OFF                 |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | PMOD PWR SELECT  : Jumper connects 1-3 |
        |               |                  : Jumper connects 2-4 |
        |               | SW_GPIO_CAN_PMOD : 2-3                 |
        |               |                  : 5-6                 |
        |               | SW_PMOD0_PWR_SLP : 2-3                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C_PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD LED to PMOD1_3A (PIN7-12) on the SMARC Carrier Board for LED output control.
    (4) Connect PMOD0_2A PIN9 to PMOD0_2A PIN7 on SMARC Carrier Board using jumper wire.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08008848
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
3. User can monitor the GPT signals using a logic analyzer to observe the effects of POEG output disable and
   POEG reset operations.
      GPT13 (used for POEG trigger pin and POEG software API modes): PMOD0_2A Pin 4 and PMOD0_2A Pin 1
      GPT4 (used for POEG trigger GPT output level mode): PMOD1_6A Pin 7 and PMOD1_6A Pin 8
