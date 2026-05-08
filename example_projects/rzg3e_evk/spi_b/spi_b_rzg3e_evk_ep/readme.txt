/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the SPI_B HAL module APIs. The project configure 
    RSPI channel 0 in Master and Pmod SF3 in Slave. Once the module is initialised and the channels are 
    configured, Master can write data to Slave and read it back based on commands from user sent through 
    J-Link RTT Viewer.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disabled on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.1.0
    e2 studio: Version 2026-04.1
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/G3E-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x PMOD SF3 (e.g., https://digilent.com/reference/pmod/pmodsf3/start).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+========================================+
        | Board setting | RZ/G3E Evaluation Board Kit            |
        +===============+========================================+
        | Module board  | BOOT-1           : OFF                 |
        |               | BOOT-2           : OFF                 |
        |               | BOOT-3           : OFF                 |
        |               | BOOT-4           : OFF                 |
        |               | BOOT-5           : OFF                 |
        |               | BOOT-6           : OFF                 |
        +---------------+----------------------------------------+
        | Carrier board | SW_MODE-1        : OFF                 |
        |               | SW_MODE-2        : ON                  |
        |               | SW_MODE-3        : OFF                 |
        |               | SW_MODE-4        : ON                  |
        |               | PMOD PWR SELECT  : Jumper connects 1-3 |
        +---------------+----------------------------------------+

5. Hardware Connections:
    (1) Connect the USB-C_PWR_IN port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD SF3 to PMOD0_2A on the carrier board.
        RZ/G3E Evaluation Kit (SMARC):
        +------------------------+------------------------+
        |    Slave (PMOD SF3)    |  Master (PMOD0_2A)     |
        +------------------------+------------------------+
        |        1 -- SSL      <-|->    1 -- P15_3        |
        |        2 -- MOSI     <-|->    2 -- P15_1        |
        |        3 -- MISO     <-|->    3 -- P15_2        |
        |        4 -- CK       <-|->    4 -- P15_0        |
        |        5 -- GND      <-|->    5 -- GND          |
        |        6 -- VCC      <-|->    6 -- VCC          |
        +------------------------+------------------------+
Note:
1. User is expected to enter data of size not exceeding above 256 bytes.
   Operation is not guaranteed for any user input value other than integer, char (i.e. float, special char)
   through RTT.
2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x08009508
3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
