/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the RSPI HAL module APIs. The project configure 
    RSPI channel 0 in Master and Pmod SF3 in Slave. Once the module is initialised and the channels are 
    configured, Master can write data to Slave and read it back based on commands from user sent through 
    JLink RTT Viewer.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please invalid the DMAC feature by the following steps.
    - Set DMAC Support to Disabled on the main stack of the driver in FSP Configuration. 
    - Remove the DMAC sub stacks that linked to the main stack.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    
3. Hardware Requirements:
    RZ supported boards: RZ/G3S-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB PD Charger Anker PowerPort III 65W Pod.
    1 x PMOD SF3 module.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (SCIF Downloading) with CA55 cold boot.

    (2) Set for DIP switches and jumpers as follow.

        +===============+====================================+
        | Board setting | RZ/G3S Evaluation Kit (SMARC)      |
        +===============+====================================+
        | Module board  | SW_CONFIG-1  : OFF                 |
        |               | SW_CONFIG-2  : OFF                 |
        |               | SW_CONFIG-3  : ON                  |
        |               | SW_CONFIG-4  : OFF                 |
        |               | SW_CONFIG-5  : OFF                 |
        |               | SW_CONFIG-6  : OFF                 |
        +---------------+------------------------------------+
        | Carrier board | SW_MODE-1    : OFF                 |
        |               | SW_MODE-2    : ON                  |
        |               | SW_MODE-3    : OFF                 |
        |               | SW_MODE-4    : ON                  |
        |               |                                    |
        |               | PMOD_PWR_SEL : Jumper connects 1-3 |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a USB PD charger using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect Pmod SF3 and Board as follows.
        +----------+-----------------------+-----------------+
        | RSPI Pin |        Board          |    Pmod SF3     |
        +----------+-----------------------+-----------------+
        |   MISO   |    PMOD0-2A Pin3   <--|-->    Pin3      |
        |   MOSI   |    PMOD0-2A Pin2   <--|-->    Pin2      |
        |   CK     |    PMOD0-2A Pin4   <--|-->    Pin4      |
        |   SSL    |    PMOD0-2A Pin1   <--|-->    Pin1      |
        +----------+-----------------------+-----------------+

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x00029308
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).
3. The user must enter data of no more than 254 bytes. Operation is only guaranteed for integer input;
   other input types (e.g., float, characters, or special characters) are not supported when entered via RTT.