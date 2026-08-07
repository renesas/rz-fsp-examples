/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the UART HAL module APIs.
    The project initializes the UART with Baud rate of 115200 bps and GTM.
    Using a Terminal Emulator user can provide a value & press enter key to set the cycle of the USER LED signal.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disable on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack.

    Steps to enable again the DMAC feature after disabled it:
    - Set DMAC Support to Enable on the main stack of the driver in FSP Configuration.
    - Add Transfer Driver on r_dmac in the DMAC sub stacks that linked to the main stack.
    - In the g_transfer0 SCIg_TXI0, set DMA Activation Request Source Select to Requested by 
      a transfer destination module.
    - In the g_transfer1 SCIg_RXI0, set DMA Activation Request Source Select to Requested by 
      a transfer source module.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: EK-RZ/A3M.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB micro-B cable.
    1 x PmodUSBUART (e.g., https://digilent.com/reference/pmod/pmodusbuart/start).
    4 x jumper wires.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 4 (3.3-V Single or Quad serial NOR flash memory).

    (2) Set for DIP switches as follow to select boot mode 4 and peripheral pins:
        +===============+==========================+
        | Board setting | EK-RZ/A3M                |
        +===============+==========================+
        | Module board  | SW4-1  : OFF (Default)   |
        |               | SW4-2  : OFF (Default)   |
        |               | SW4-3  : OFF (Default)   |
        |               | SW4-4  : OFF (Default)   |
        |               | SW4-5  : OFF             |
        |               |                          |
        |               | SW5-1  : OFF             |
        |               | SW5-2  : OFF             |
        |               | SW5-3  : OFF (Default)   |
        |               | SW5-4  : OFF             |
        |               | SW5-5  : OFF (Default)   |
        +---------------+--------------------------+

    (3) The user needs to place jumper J9 on pins 2-3 to use on-board debug functionality.

5. Hardware Connections:
    (1) Connect the DEBUG1 port (J10) of the RZ board to the PC using a USB Type-C cable for board power supply, debugging,
        and serial terminal communication (Tera Term).

    (2) Connect J1 on EK-RZ/A3M Board and J2 on PmodUSBUART with jumper wire as follows.

        +-----------------+-----------------------+
        |  J2 PmodUSBUART |J1 (EK-RZ/A3M Board)   |
        +-----------------+-----------------------+
        | 1 --- NC        |                       |
        | 2 --- RXD    <--|-->  P4_3 (SCI0_TXD)   |
        | 3 --- TXD    <--|-->  P4_4 (SCI0_RXD)   |
        | 4 --- NC        |                       |
        | 5 --- GND    <--|-->  GND               |
        | 6 --- VCC    <--|-->  +3.3V             |
        +-----------------+-----------------------+

    (3) Connect Connector (J1) on PmodUSBUART to PC through USB micro-B cable.

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none