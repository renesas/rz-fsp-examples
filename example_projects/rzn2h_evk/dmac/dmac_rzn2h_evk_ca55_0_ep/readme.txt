/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates a typical use case of the DMAC module. It initializes the DMAC functionality 
    and performs data transfers from a source array located in ATCM to a destination array.
    The transfers are triggered using both a software trigger and a CMTW compare match event.
    Upon successful completion, the received data is displayed in the Tera Term.
    Any API/event failure message is also displayed.

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
    1 x USB Type-A to USB Mini-B cable

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:
    The operating mode of the RZ/N2H
        DSW3.1   : ON
        DSW3.2   : OFF
        DSW3.3   : ON

    CPU0 ATCM wait cycle = 1 wait cycle
        DSW3.4   : OFF

    Use LED8
        DSW15.8  : OFF
        DSW15.9  : OFF
        DSW15.10  : ON

    Use UART data for USB-to-serial conversion
        DSW9.1 : ON
        DSW9.2 : OFF
        DSW9.3 : ON
        DSW9.4 : OFF
    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN27 - USB Type-A to USB Mini-B cable
          CN26 - USB Type-A to micro USB cable.