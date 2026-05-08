/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of the i2c slave HAL module APIs. The project initializes both the 
    I2C slave and I2C master module with standard rate and is made interfaced with loop-back mechanism. It performs 
    slave read and write operation continuously once initialization is successful.
    On successful I2C transaction (6 bytes), Data transceived is compared. LED blinks on data match else it is turned
    on as sign of failure. Output message for both corresponding slave operations is displayed on RTT Viewer.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    2 x Jumper wires.
    1 x PMOD LED.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+=====================================+
        | Board setting | RZ/G3E Evaluation Board Kit         |
        +===============+=====================================+
        | Module board  | SW5-1  : OFF                        |
        |               | SW5-2  : OFF                        |
        |               | SW5-3  : OFF                        |
        |               | SW5-4  : OFF                        |
        |               | SW5-5  : OFF                        |
        |               | SW5-6  : OFF                        |
        +---------------+-------------------------------------+
        | Carrier board | SW_MODE-1    : OFF                  |
        |               | SW_MODE-2    : ON                   |
        |               | SW_MODE-3    : OFF                  |
        |               | SW_MODE-4    : ON                   |
        |               | PMOD_PWR_SEL : Jumper connects 1-3  |
        |               | PMOD_PWR_SEL : Jumper connects 2-4  |
        +---------------+-------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C_PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for board 
        power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect Pin 7 (PMOD0_2A) to  Pin 4 (PMOD1_6A) on SMARC Carrier Board.
        Connect Pin 8 (PMOD0_2A) to  Pin 3 (PMOD1_6A) on SMARC Carrier Board.
    (4) Connect Pmod LED to PMOD1_3A (pin 7-12) on SMARC Carrier Board.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08009068

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
