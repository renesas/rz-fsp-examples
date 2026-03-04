/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the IIC master HAL module APIs.
    The project initializes IIC master module with fast mode and interfaces with PmodACL™ Board for ADXL345.

    On power up after establishing the connection of sensor with the board, it displays accelerometer axis data on
    Tera Term. Any API/event failure will be displayed on Tera Term.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.
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

        - Use UART data for USB-to-serial conversion
          DSW9.1  : ON
          DSW9.2  : OFF
          DSW9.3  : ON
          DSW9.4  : OFF

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
          CN27 - USB Type-A to USB Mini-B cable
          CN26 - USB Type-A to micro USB cable.