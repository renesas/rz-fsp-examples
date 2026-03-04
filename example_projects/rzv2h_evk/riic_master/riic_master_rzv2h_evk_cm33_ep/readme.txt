/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the IIC master HAL module APIs.
    The project initializes IIC master module with fast mode and interfaces with PmodACL™ Board for ADXL345.

    On power up after establishing the connection of sensor with the board, it displays accelerometer axis data on 
    RTTviewer. Any API/event failure will be displayed on RTTviewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x Pmod ACL.

4. Hardware Configurations:
    (1) Boot Mode:
        CA55 coldboot (SCIF boot).
    (2) Set for DIP switches and jumpers as follow.

        +===============+====================================+
        | Board setting | RZ/V2H Evaluation Board            |
        +===============+====================================+
        | Module board  | DSW1-1  : ON                       |
        |               | DSW1-2  : OFF                      |
        |               | DSW1-3  : ON                       |
        |               | DSW1-4  : OFF                      |
        |               | DSW1-5  : ON                       |
        |               | DSW1-6  : OFF                      |
        |               | DSW1-7  : ON                       |
        |               | DSW1-8  : OFF                      |
        +---------------+------------------------------------+
        | Others        | Default                            |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN13 on the RZ/V2H Evaluation Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD ACL with RZV2H EVK Expansion Board as follows.
        PMOD ACL has two on board connectors, J2 is used for I2C communication.
        RIICx_SCL/PMOD Type6/6A pin 3  ----> SCL (J2:5 on PMOD-ACL)
        RIICx_SDA/PMOD Type6/6A pin 4  ----> SDA (J2:6 on PMOD-ACL)
        GND/PMOD Type6/6A pin 5        ----> GND (J2:7 on PMOD-ACL)
        VCC/PMOD Type6/6A pin 6        ----> VCC (J2:8 on PMOD-ACL)

Note:
1. With the SDO/ALT ADDRESS pin high, the 7-bit I2C address for the device is 0x1D, followed by the R/W bit.
   An alternate I2C address of 0x53 (followed by the R/W bit) can be chosen by grounding the SDO/ALT ADDRESS pin.
2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x0800e730
3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).