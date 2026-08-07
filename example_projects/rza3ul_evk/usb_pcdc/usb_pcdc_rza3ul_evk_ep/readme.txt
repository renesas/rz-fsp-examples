/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the basic functionalities of the USB PCDC driver on Renesas RZA MPUs based
    on Renesas FSP.
        - On power up or RESET, all LEDs on PMOD LED start blinking at 1Hz.
        - Press BTN0 button of PMOD BTN to change the blinking frequency of the PMOD LED. With every press BTN0
          button, the frequency will switch from 1 Hz to 5 Hz, then to 10 Hz, and cycle back.
        - Open the serial port of the device using Terminal Emulator and press Enter key (Keyboard on host PC).
            + Press 1: Show kit information (Show kit name, blinking frequency).
            + Press 2: Show URLs to the user visit.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Micro-B cable.
    1 x Pmod BTN (e.g., https://digilent.com/reference/pmod/pmodbtn/start).
    1 x Pmod LED (e.g., https://digilent.com/reference/pmod/pmodled/start).

    RZ supported boards:
        RZ/A3UL Evaluation Board Kit QSPI Edition (Exec with DDR SDRAM).
        RZ/A3UL Evaluation Board Kit QSPI Edition (eXecute-In-Place).
        RZ/A3UL Evaluation Board Kit OCTAL Edition (eXecute-In-Place).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).

    (2) Set for DIP switches and jumpers as follow:
        +===============+====================================+
        | Board setting | RZ/A3UL Evaluation Board Kit       |
        +===============+====================================+
        | Module board  | SW1-1   : OFF                      |
        |               | SW1-2   : Don't care               |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Don't care               |
        |               | SW2     : Don't care               |
        |               | SW3     : Don't care               |
        |               | SW4     : Don't care               |
        |               | SW5     : 3-2                      |
        |               | SW6     : Don't care               |
        |               | SW7     : Don't care               |
        |               | SW8     : Don't care               |
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
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect the PMOD BTN to the PMOD1 CN7 (7:12).
    (4) Connect the PMOD LED to the PMOD0 J1 (7:12).
    (5) Connect the USB 0 (OTG) (CN11) on SMARC Carrier Board to the host PC using a USB Micro-B cable.

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none