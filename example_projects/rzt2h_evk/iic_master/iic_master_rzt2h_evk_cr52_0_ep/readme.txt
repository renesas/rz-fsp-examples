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
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x Pmod ACL.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:
        - The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
          SW14.1 : ON
          SW14.2 : OFF
          SW14.3 : ON
          SW14.6 : OFF

        - CPU0 ATCM wait cycle = 1 wait cycle
          SW14.4 : OFF

        - Configuring the EVK board circuit
          Connect IIC(Slave) to CN53 and CN54 of mikroBUS™.
          Pins : Master side   - Slave side
          ------------------------------------
          SCL  : CN54.5(P04_7) - J2.5
          SDA  : CN54.6(P05_0) - J2.6
          p3v3 : CN53.7        - J2.8
          GND  : CN53.8        - J2.7
          ------                       -------
          ( RZ/T2H EVK uses IIC1)

    Configuring the Evaluation board circuit
        - USB connection between Evaluation board and PC
          CN34 - USB Type-A to USB Mini-B cable
          CN14 - USB Type-A to micro USB cable

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).