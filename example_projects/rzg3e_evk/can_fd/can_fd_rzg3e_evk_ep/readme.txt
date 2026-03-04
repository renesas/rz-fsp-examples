/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates CANFD operations on Renesas RZ MPUs using 1 RZ/G board.
    On pressing any key on the RTT Viewer, data is transmitted from one channel to other.
    On 1st transmission, channel 4 transmits data to channel 1. Channel 1 displays the received data.
    On 2nd transmission, channel 1 transmits updated data to channel 4 as ACK. Channel 4 displays the received data,
    then channel 4 changes CAN frame to CANFD frame and updates data.
    On 3rd transmission, channel 4 transmits updated data to channel 1. Channel 1 displays the received data,
    then channel 1 changes CAN frame to CANFD frame and updates data.
    On 4th transmission, channel 1 transmits updated data to channel 4 as ACK. Channel 4 displays the received data.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD LED module.
    1 x CAN wire.

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
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | SW_M2_DIS-4      : ON                  |
        |               | SW_GPIO_CAN_PMOD : 2-3                 |
        |               |                  : 5-6                 |
        |               | PMOD PWR SELECT  : Jumper connects 2-4 |
        |               | SW_GPIO_OPT_SEL  : 2-3                 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD1-3A pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control. 
    (4) Connect CAN0 to CAN1 (CAN_H, CAN_L, GND) for CAN communication.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x080094b0
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
3. Board RZ/G Evaluation Board Kit - must have IC41 (to support CAN port) is on the SMARC Carrier Board.
4. The user must enter data of no more than 15 bytes.
