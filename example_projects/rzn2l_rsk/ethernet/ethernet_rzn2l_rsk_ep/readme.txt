/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This software is a sample program designed to demonstrate ping response functionality using the Ethernet driver for the RZ/N2L device.
    By default, it enables Ethernet communication via three ports: ETH0, ETH1, and ETH2 on the RZ/N2L board.
    To evaluate simultaneous 3-port Ethernet communication, the host PC must have three available Ethernet interfaces, 
    each connected to a corresponding port on the board.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/N2L-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    3 x Ethernet cables.

4. Hardware Configurations:
    (1) Boot Mode:
        16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON
        SW4.4 : ON
        SW4.5 : OFF

        2. Set SW8 following.
        SW8.1 : OFF
        SW8.2 : ON
        SW8.3 : OFF
        SW8.4 : OFF
        SW8.5 : ON
        SW8.6 : OFF
        SW8.7 : ON
        SW8.8 : OFF
        SW8.9 : OFF
        SW8.10: ON

        3. Set SW11 following.
        SW11.1 : ON
        SW11.2 : OFF
        SW11.3 : OFF
        SW11.4 : OFF
        SW11.5 : ON
        SW11.6 : OFF
        SW11.7 : ON
        SW11.8 : OFF
        SW11.9 : OFF
        SW11.10: ON

        4. Set Connectors following.
        CN17:2-3 short
        CN20:1-2 short
        CN21:1-2 short
        CN22:1-2 short
        
        5. Connect ETH0, ETH1, and ETH2 ports on the RSK board to three separate Ethernet ports on the host PC using Ethernet cables.

Note:
1. The host PC must have three Ethernet interfaces configured with static IPs on the same subnet as the board:
     ETH0 <-> 192.168.10.1 (PC side), board = 192.168.10.100
     ETH1 <-> 192.168.20.1 (PC side), board = 192.168.20.100
     ETH2 <-> 192.168.30.1 (PC side), board = 192.168.30.100

2. How to set on the host PC (generic steps):
    (1) Open the operating system’ network settings and identify the Ethernet adapter that is physically connected to ETH0 on the board.
    (2) Edit that adapter’s IPv4 settings and configure the following:
        IP address: 192.168.10.1
        Subnet mask: 255.255.255.0
        Default gateway: leave empty (not required for local ping test)
    (3) Repeat the above steps for the adapters connected to ETH1 and ETH2, using IP addresses 192.168.20.1 and 192.168.30.1 respectively.

3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000.

4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).