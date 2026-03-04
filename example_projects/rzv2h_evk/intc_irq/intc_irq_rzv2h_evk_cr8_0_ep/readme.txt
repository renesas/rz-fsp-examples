/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
     The example project demonstrates the typical use of the INTC IRQ module APIs.
     The project initializes the IRQ interrupt in Interrupt Controller 
     User is requested to press the push button to trigger the external IRQ and this then will start toggling of user LED.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD BUTTON module.

4. Hardware Configurations:
    (1) Boot Mode:
        CA55 coldboot (SCIF boot).
    (2) Set for DIP switches and jumpers as follow:
        +===============+====================================+
        | Board setting | RZ/V2H Evaluation Board            |
        +===============+====================================+
        | Module board  | DSW1-1  : ON                       |
        |               | DSW1-2  : OFF                      |
        |               | DSW1-3  : ON                       |
        |               | DSW1-4  : OFF                      |
        |               | DSW1-5  : ON                       |
        |               | DSW1-6  : OFF                      |
        |               | DSW1-7  : ON                       |
        |               | DSW1-8  : OFF                      |
        +---------------+------------------------------------+
        | Others        | Default                            |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN13 on the RZ/V2H Evaluation Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD BUTTON to Pmod Type1/1A(CN1) (pin 1-6) on RZ/V2H EVK Expansion Board.

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08182e34
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).