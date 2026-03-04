/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This sample program demonstrates how to use the UART module to send and receive messages via the terminal on the RZ/T2H
    micro controller. The project initializes the UART with baud rate of 115200 bps. Open Serial Terminal with this baud rate 
    value and transmit a 'Hello World' message to the terminal. It then enters a receive loop, waiting for user input.
    Any data received from the terminal is immediately echoed back.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.

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

        - Use UART data for USB-to-serial conversion
          SW4.5  : ON
          SW4.6  : OFF
          SW4.7  : ON
          SW4.8  : OFF

    Configuring the Evaluation board circuit
        - USB connection between Evaluation board and PC
          CN34 - USB Type-A to USB Mini-B cable
          CN14 - USB Type-A to micro USB cable