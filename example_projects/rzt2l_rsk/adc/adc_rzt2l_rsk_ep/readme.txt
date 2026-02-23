/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the ADC HAL module APIs.
    The project initializes the ADC in single scan or continuous scan mode based on user selection in RZ configurator.
    Once initialized, user can initiate the ADC scan and also stop the scan (in the case of continuous scan mode) using 
    JLinkRTTViewer by sending commands. User provide ADC channel input voltage from 0V to 1.8V with a on board Potentiometer R5
    at the ADC channel voltage input pin AN000. Once ADC scan is initialized, Window Compare Mode is enabled and compares the 
    ADC channel input voltage with the upper and lower limits. The upper limit and lower limit are configured in RZ 
    Configurator. If the ADC channel input voltage is above the upper limit or below the lower limit, it triggers an event 
    and notifies the user to act accordingly. Result and ADC status are displayed on the JLinkRTTViewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2L-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI0 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *SW4 settings must be below to start this sample program.
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.4  : OFF

4. Configuration Settings:
    The example project, by default, runs in Single scan mode on the board.
    In order to switch the scan mode, change the 'Mode' field of ADC Driver in the RZ configurator to Continuous Scan.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).