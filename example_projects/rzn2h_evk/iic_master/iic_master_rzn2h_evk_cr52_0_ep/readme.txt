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
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x Pmod ACL.
 
4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:
        - The operating mode of the RZ/N2H: xSPI1 boot mode (x1 boot serial flash)
          DSW3.1   : ON
          DSW3.2   : OFF
          DSW3.3   : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4   : OFF

        - Use IIC(Master)
          DSW7.1   : ON
          DSW7.2   : OFF
          DSW7.3   : ON
          DSW7.4   : OFF

        - Connect Pmod ACL and RZ/N2H Board as follows
          ------------------------------------
          J2 (Pmod ACL)       CN29 Grove (I2C)
          |1  5|---SCL  --->  CN29.1
          |2  6|---SDA  --->  CN29.2
          |3  7|---GND  --->  CN29.4
          |4  8|---VCC  --->  CN29.3
          ------------------------------------
          ( RZ/N2H EVK uses IIC1)

    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN26 - USB Type-A to micro USB cable.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).