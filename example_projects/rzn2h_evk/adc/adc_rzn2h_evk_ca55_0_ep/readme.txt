/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ADC HAL module APIs.
    The project initializes the ADC in single scan or continuous scan mode based on user selection in RZ configurator.
    Once initialized, user can initiate the ADC scan and also stop the scan (in the case of continuous scan mode) using 
    Tera Term by sending commands. User provide ADC channel input voltage from 0V to 1.8V with a on board Potentiometer R66
    at the ADC channel voltage input pin AN000. Once ADC scan is initialized, Window Compare Mode is enabled and compares the 
    ADC channel input voltage with the upper and lower limits. The upper limit and lower limit are configured in RZ 
    Configurator. If the ADC channel input voltage is above the upper limit or below the lower limit, it triggers an event 
    and notifies the user to act accordingly. Result and ADC status are displayed on the Tera Term.

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
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

        - Boot mode switch:
          DSW3.1 : ON
          DSW3.2 : OFF
          DSW3.3 : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4 : OFF

        - Use Potentiometer(R66)
          DSW6.1 : OFF
          DSW6.2 : ON

        - Use UART data for USB-to-serial conversion
          DSW9.1 : ON
          DSW9.2 : OFF
          DSW9.3 : ON
          DSW9.4 : OFF

    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN27 - USB Type-A to USB Mini-B cable
          CN26 - USB Type-A to micro USB cable.

4. Configuration Settings:
    The example project, by default, runs in Single scan mode on the boards.
    In order to switch the scan mode, change the 'Mode' field of ADC Driver in the RZ configurator to Continuous Scan.