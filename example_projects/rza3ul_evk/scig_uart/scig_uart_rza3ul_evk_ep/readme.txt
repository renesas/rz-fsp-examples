/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the UART HAL module APIs.
    The project initializes the UART with Baud rate of 115200 bps and GTM.
    Using a Terminal Emulator user can provide a value & press enter key to set the cycle of the Pmod LED signal.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disable on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack .

    Steps to enable again the DMAC feature after disabling it:
    - Set DMAC Support to Enable on the main stack of the driver in FSP Configuration. 
    - Add Transfer Driver on r_dmac in the DMAC sub stacks that linked to the main stack
    - In the g_transfer0 SCIg_TXI0, set DMA Activation Request Source Select to Requested by a transfer destination module
    - In the g_transfer1 SCIg_RXI0, set DMA Activation Request Source Select to Requested by a transfer source module

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.
    1 x PMOD LED module.
    1 x PmodUSBUART

    RZ supported boards:
        RZ/A3UL Evaluation Board Kit QSPI Edition  (Exec with DDR SDRAM).
        RZ/A3UL Evaluation Board Kit QSPI Edition  (eXecute-In-Place).
        RZ/A3UL Evaluation Board Kit OCTAL Edition (eXecute-In-Place).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).

    (2) Set for DIP switches and jumpers as follow:

        +===============+====================================+
        | Board setting | RZ/A3UL Evaluation Board Kit       |
        +===============+====================================+
        | Module board  | SW1-1   : OFF                      |
        |               | SW1-2   : Default                  |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Default                  |
        |               | SW2     : Default                  |
        |               | SW3     : Default                  |
        |               | SW4     : Default                  |
        |               | SW5     : 1-2                      |
        |               | SW6     : 1-2                      |
        |               | SW7     : 1-2                      |
        |               | SW8     : 1-2                      |
        |               |                                    |
        |               | SW11-1  : OFF                      |
        |               | SW11-2  : OFF                      |
        |               | SW11-3  : OFF                      |
        |               | SW11-4  : ON                       |
        |               |                                    |
        |               | CN4     : Jumper connects 1-3      |
        |               |           Jumper connects 2-4      |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMRAC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect the CN14 port on the Smarc Carrier Board to the PC using a USB Type-microB cable for serial terminal
        communication (Tera Term).
    (4) Connect with PC through USB Type-microB Connector (J1) on PmodUSBUART.
    (5) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control.
    (6) Connect PMOD1 on SMARC Carrier Board and PmodUSBUART with jump wire as follows.
        
        +------------------------+------------------------+
        |    J2 (PmodUSBUART)    |  PMOD1 (SMARC Carrier) |
        +------------------------+------------------------+
        |        1 -- NC         |                        |
        |        2 -- RXD      <-|->    7 -- SCI0_TXD     |
        |        3 -- TXD      <-|->    8 -- SCI0_RXD     |
        |        4 -- NC         |                        |
        |        5 -- GND      <-|->    11 - GND          |
        |        6 -- VCC      <-|->    12 - VCC          |
        +------------------------+------------------------+