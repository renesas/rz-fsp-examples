/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CAN-FD running on Renesas RZ MPUs using channel 0 and channel 1 on RZ/G board. 
    On pressing any key on the Terminal Emulator, data is transmitted from one channel to other.
    On 1st transmission, Channel 0 transmits data to Channel 1. Channel 1 displays the received data.
    On 2nd transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data
    then Channel 0 changes CAN frame to CANFD frame and updates data.
    On 3rd transmission, Channel 0 transmits updated data to Channel 1. Channel 1 displays the received data
    then Channel 1 changes CAN frame to CANFD frame and updates data to transmits back to Channel 0 as ACK.
    On 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3S-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB PD Charger Anker PowerPort III 65W Pod.
    1 x PMOD LED module.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+========================================+
        | Board setting | RZ/G3S Evaluation Kit (SMARC)          |
        +===============+========================================+
        | Module board  | SW_CONFIG-1      : OFF                 |
        |               | SW_CONFIG-2      : OFF                 |
        |               | SW_CONFIG-3      : ON                  |
        |               | SW_CONFIG-4      : OFF                 |
        |               | SW_CONFIG-5      : OFF                 |
        |               | SW_CONFIG-6      : OFF                 |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | SW_M2_DIS-4      : ON                  |
        |               | SW_GPIO_CAN_PMOD : 2-3                 |
        |               |                  : 5-6                 |
        |               | PMOD_PWR_SEL     : Jumper connects 2-4 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1-3A pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control. 
    (4) Connect CAN0 to CAN1 (CAN_H, CAN_L, GND) for CAN communication.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x00028b18
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
3. Board RZ/G Evaluation Board Kit - must have IC41 (to support CAN port) is on the carrier board.