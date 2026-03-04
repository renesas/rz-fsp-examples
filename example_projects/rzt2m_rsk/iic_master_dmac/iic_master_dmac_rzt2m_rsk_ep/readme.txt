/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates the use of the IIC Master module with DMAC support on the RZ/T2M device..
    The project initializes the IIC master module at a standard rate and communicates with the PmodACL™ Board featuring the ADXL345 accelerometer.

    Upon power-up, after establishing the sensor connection with the RZT board, the system displays real-time accelerometer axis data via RTTviewer.
    Any API or event failure will also be reported on RTTviewer for debugging purposes.

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
    1 x Pmod ACL.

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

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. *Set SW6 following.
        SW6.1 : OFF

        4. Connect Pmod ACL and RSK + RZT2M Board as follows.
        Pmod ACL has two on board connectors, J2 is used for I2C communication.
        ------------------------------------
        J2 (Pmod ACL)       J27 Grove (I2C)
        |1  5|---SCL  --->  J27.1
        |2  6|---SDA  --->  J27.2
        |3  7|---GND  --->  J27.4
        |4  8|---VCC  --->  J27.3
        ------------------------------------
        ( RZ/T2M RSK uses IIC1)

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000
 
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
