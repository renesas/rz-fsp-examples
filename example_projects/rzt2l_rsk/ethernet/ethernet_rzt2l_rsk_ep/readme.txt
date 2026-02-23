/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This software is a sample program designed to demonstrate ping response functionality using the Ethernet driver for the RZ/T2L device.
    By default, it enables Ethernet communication via Ethernet port 2 (ETH2) on the RZ/T2L board.
    To evaluate simultaneous Ethernet communication, the host PC must have one available Ethernet interface, 
    it connected to a corresponding port on the board.

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
    1 x Ethernet cable.

4. Hardware Configurations:
    (1) Boot Mode:
        16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON
        SW4.4 : OFF
        SW4.5 : ON
        SW4.6 : ON
        SW4.7 : OFF
        SW4.8 : OFF

        2. Set SW5 following.
        SW5.1 : ON
        SW5.2 : OFF
        SW5.3 : OFF
        SW5.4 : OFF
        SW5.5 : OFF
        SW5.6 : OFF
        SW5.7 : OFF
        SW5.8 : OFF
        SW5.9 : ON
        SW5.10: OFF

        3. Set SW6 following.
        SW6.1 : ON
        SW6.2 : OFF
        SW6.3 : OFF
        SW6.4 : OFF
        SW6.5 : OFF
        SW6.6 : OFF
        SW6.7 : OFF
        SW6.8 : ON
        SW6.9 : OFF
        SW6.10: OFF

        4. Set SW7 following.
        SW7.1 : OFF
        SW7.2 : OFF
        SW7.3 : ON
        SW7.4 : OFF
        SW7.5 : OFF
        SW7.6 : ON
        SW7.7 : OFF
        SW7.8 : OFF
        SW7.9 : OFF
        SW7.10: ON

        5. Set SW8 following.
        SW8.1 : ON
        SW8.2 : OFF
        SW8.3 : OFF
        SW8.4 : OFF
        SW8.5 : ON
        SW8.6 : OFF
        SW8.7 : OFF
        SW8.8 : OFF
        SW8.9 : ON
        SW8.10: OFF

        6. Set Connectors following.
        CN17:2-3 short
        CN32:1-2 short
        CN33:1-2 short
        CN34:1-2 short
        CN35:1-2 short
        CN18: short
        CN23: short

        5. Connect ETH2 port on the RSK board to Ethernet port on the host PC using Ethernet cable.

Note:
1. The host PC must have one Ethernet interface configured with static IPs on the same subnet as the board:
   ETH2 <-> 192.168.10.1 (PC side), board = 192.168.10.100

2. How to set on the host PC (generic steps):
    (1) Open the operating system’ network settings and identify the Ethernet adapter that is physically connected to ETH2 on the board.
    (2) Edit that adapter’s IPv4 settings and configure the following:
        IP address: 192.168.10.1
        Subnet mask: 255.255.255.0
        Default gateway: leave empty (not required for local ping test)

3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).