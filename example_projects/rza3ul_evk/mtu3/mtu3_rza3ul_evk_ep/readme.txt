/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of MTU3 HAL module APIs. User has the provision to input value as per
    displayed menu through a terminal application to select different MTU3 supported modes(Periodic, PWM, One-Shot). In
    periodic mode, user can enter the time period within the permitted ranges to change the frequency of the user LED.
    In PWM mode, user can enter the duty cycle within the specified range to adjust the intensity of the user LED.
    In One-Shot mode, output will be displayed on a terminal application. Any failure will also be displayed on a
    terminal application.


2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirement:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.
    1 x PMOD LED module.

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
        |               | SW5     : Default                  |
        |               | SW6     : Default                  |
        |               | SW7     : Default                  |
        |               | SW8     : Default                  |
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
    (4) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control.

Note:
    1.Timer period range in millisecond as EP configuration:
        - RZ/A3UL    --    (1 to 1342) ms

    2.Operation is not guaranteed for any user input value other than integer(i.e. float, char, special char) through
      a terminal application.