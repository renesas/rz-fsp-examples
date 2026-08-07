/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the use of the Real-Time Clock (RTC) on the
    RZ/N2H microprocessor. It initializes the RTC module with a start calendar time and enables.
    A 1-second periodic interrupt. On each periodic interrupt, the current calendar time is read
    and displayed via the Tera Term.
    A hardware alarm is configured to trigger when the hour and minute match and
    the current day-of-week is enabled. Any API failure is also displayed on the console.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        - The operating mode of the RZ/N2H : xSPI1 boot mode (x1 boot serial flash)
          DSW3.1   : ON
          DSW3.2   : OFF
          DSW3.3   : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4   : OFF

        - Use UART data for USB-to-serial conversion
          DSW9.1   : ON
          DSW9.2   : OFF
          DSW9.3   : ON
          DSW9.4   : OFF

5. Hardware Connections:
    (1) Connect the CN26 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN27 port on evaluation board to the PC using USB Mini-B cable for serial terminal 
        communication (Tera Term).
    (3) Connect the CN13 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (4) Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
	
Note:
1. The configuration parameters of the serial port are:
        - COM port: Provided by the J-Link OB VCOM.
        - Speed 115200
        - Data: 8 bit
        - Parity: none
        - Stop bits: 1 bit
        - Flow control: none 