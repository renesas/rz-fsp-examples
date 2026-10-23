/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates how to use the SCIE UART module to send and receive
    messages via the terminal on the RZ/T2N microprocessor. The project initializes
    the UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate
    value and transmits a 'Hello World' message to the terminal. It then enters a
    receive loop, waiting for user input. Any data received from the terminal is
    immediately echoed back.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    FSP Smart Configurator: Version 2026-07
    IAR: Version 9.60.3
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2N-EVK.
    1 x Renesas RZ board.
    1 x USB Type-C cable.
    1 x PMOD UART module (e.g., https://digilent.com/reference/pmod/pmodusbuart/start).
    2 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).

    (2) Set for DIP switches and jumpers as follow.
        Hardware Connection:
        - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
            SW8.1 : ON
            SW8.2 : OFF
            SW8.3 : ON

        - 2nd Boot CPU : Cortex-A55 Core0
            SW10.10 : OFF

        - Use P16_2/P16_3 (PMOD1 connector, J34 pins 9/10) as SCIE4 UART
            SW7.5   : OFF

5. Hardware Connections:
    (1) Connect the J7 port on evaluation board to a 15V3A power source using a USB Type-C cable for board power supply.
    (2) Connect the J32 port on evaluation board to the PC using micro USB cable for debugging.
    (3) Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.
    (4) Connect PMOD UART and RZ/T2N-EVK as follows.
        J2 pin 3 (TXD) --> J34 pin 9  (RXDE4)
        J2 pin 2 (RXD) --> J34 pin 10 (TXDE4)
        J2 pin 5 (GND) --> J34 pin 11 (GND)
        J2 pin 6 (VCC) --> J34 pin 12 (VCC)
    (5) Connect J1 on the PMOD UART module to the PC using a USB Micro-B cable.

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none
   Enable local echo option: Setup -> Terminal -> Enable "Local echo"