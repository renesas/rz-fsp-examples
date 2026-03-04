/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the function of the r_adc_c driver. The example project demonstrates the typical
    use of the ADC HAL module APIs. The project initializes the ADC in single scan or repeat scan mode based on the user
    selection in the RZ/A3UL configuration. Once initialized, the user can initiate the ADC scan and also stop the scan
    (in the case of repeat scan mode) using Tera Term by sending commands. The user provides ADC channel input voltage
    from 0V to 1.8V using a voltage supply unit at the ADC channel voltage input pin. The result and ADC status are
    displayed on Tera Term. In repeat mode, the MTU3 module is used as a hardware trigger source for ADC conversions. To
    use this mode, open the ADC stack on fsp configuration tab, switch the conversion mode to Repeat, and set the
    trigger mode to Hardware trigger.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable.
    1 x External Variable Power Supply Unit (Rheostats 10K).

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
    (4) Connect CN1-CH0_Pin (ADC_CH0) on the SMARC Module Board to an external variable power supply (10 kΩ rheostat)
        for ADC input voltage.

Note:
    The example project, by default, runs in Single scan mode on the boards.
    To switch repeat mode, set the ADC Driver "Conversion mode" field to "Repeat mode" and the "Trigger mode" field
    to "Hardware trigger mode" in the FSP configuration tab.