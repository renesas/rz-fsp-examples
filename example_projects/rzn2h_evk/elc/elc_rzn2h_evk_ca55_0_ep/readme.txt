/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ELC module APIs.
    Input edge detection of single input port 0 and GPT9 events are linked using ELC.
    The start source for GPT9 is  ELC Single Port 0 Event. GPT9 runs in Periodic mode.
    On giving valid RTT input, a high level from single port triggers timer to start counting.
    Counter value will be displayed on Tera Term.

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

4. Hardware Configurations:
    (1) Boot Mode: 
        xSPI1 boot mode (x1 boot serial flash)
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

    1.   *Configuration of Evaluation board by Switches and Jumper Blocks
        The operating mode of the RZ/N2H
        DSW3.1   : ON
        DSW3.2   : OFF
        DSW3.3   : ON

    2. *CPU0 ATCM wait cycle = 1 wait cycle
        DSW3.4   : OFF

    3. *Configuring the board circuit
    Connect GPIO and ELC Single Port 0 pins following
        CN46.18 - CN46.13

    4. *Use UART data for USB-to-serial conversion
        DSW9.1 : ON
        DSW9.2 : OFF
        DSW9.3 : ON
        DSW9.4 : OFF
    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN27 - USB Type-A to USB Mini-B cable
          CN26 - USB Type-A to micro USB cable.
