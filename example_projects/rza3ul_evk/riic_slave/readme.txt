/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of the RIIC slave HAL module APIs on 2 Renesas RZ/A3UL MPUs.
    The project initializes RIIC slave and RIIC master module with standard rate and is made interfaced with loop-back mechanism.
    It performs Slave read and write operation continuously once initialization is successful. On successful I2C transaction
    (6 bytes), Data transceived is compared. Led blinks on data match else it is turned ON as sign of failure.
    Output message for both corresponding slave operations is displayed on Terminal Emulator.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    2 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-microB cable
    2 x PMOD LED module.

    RZ supported boards:
        RZ/A3UL Evaluation Board Kit QSPI Edition  (Exec with DDR SDRAM).
        RZ/A3UL Evaluation Board Kit QSPI Edition  (eXecute-In-Place).
        RZ/A3UL Evaluation Board Kit OCTAL Edition (eXecute-In-Place).

4. Hardware Configuration (for both) : 
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
        |               | SW3     : 1-2                      |
        |               | SW4     : 1-2                      |
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
    (4) Connect PMOD0 pins 7–12 on the SMARC Carrier Board to the PMOD LED for LED output control (for both).
    (5) Pins settings:

        +----------+-------------------+-------------------+
        | I2C Pin  |   Master board    |    Slave board    |
        +----------+-------------------+-------------------+
        |   SDA    |    PMOD1 Pin4  <--|-->  PMOD1 Pin4    |
        |   SCL    |    PMOD1 Pin3  <--|-->  PMOD1 Pin3    |
        +----------+-------------------+-------------------+


6. The user cannot change the configuration from Master to Slave at run time.
    (1) The example project riic_master_rza3ul_evk_ep has Master configuration settings and runs in master mode
        and the example project riic_slave_rza3ul_evk_ep has Slave configuration settings and runs in slave mode.

Note:
1. Need to run the example on Slave Board first, then run the example on Master board.
   Make sure the slave board in operation when start running master board.
2. The GND of the two boards must be shared or connected together.
