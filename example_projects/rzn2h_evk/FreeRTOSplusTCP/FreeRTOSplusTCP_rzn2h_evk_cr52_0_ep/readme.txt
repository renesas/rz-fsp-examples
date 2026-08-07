/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates Ethernet communication on 3 ports: ETH0, ETH1, and ETH2.
    The board uses a static IPv4 address (192.168.0.100) and supports the following operations:
      1. Ping reply check from host PC.
      2. UDP echo server test using Packet Sender (Address: 192.168.0.100, Port: 10000, ASCII text).
      3. TCP echo server test using Packet Sender (Address: 192.168.0.100, Port: 10000, ASCII text).
    For each test, verify both the host-side result and the corresponding logs on the RTT Viewer.
    Link up/down status for each Ethernet port is also monitored and printed on the console.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 9.44
    Packet Sender Software

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x Ethernet cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        1. *Boot mode switch:
        DSW3.1 : ON
        DSW3.2 : OFF
        DSW3.3 : ON

        2. *CPU0 ATCM wait cycle = 1 wait cycle
        DSW3.4 : OFF

        3. Use Ethernet ports
        JP6 : 2-3 short
        JP7 : 2-3 short
        JP8 : 1-2 short

        DSW5.6  : ON
        DSW5.7  : ON
        DSW8.1  : OFF
        DSW8.2  : ON
        DSW12.3 : OFF
        DSW12.4 : ON
        DSW13.1 : ON
        DSW13.2 : OFF
        DSW13.3 : ON
        DSW13.4 : OFF
        DSW13.5 : ON
        DSW13.6 : OFF
        DSW13.7 : OFF
        DSW13.8 : ON
        DSW21.1 : ON
        DSW21.2 : OFF
        DSW21.3 : ON 

5. Hardware Connections:
    (1) Connect the CN26 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (3) Connect the CN13 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (4) Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.

Note:
1. The host PC must have one Ethernet interface configured with static IP on the same subnet as the board:
   ETH <-> 192.168.0.1 (PC side), board = 192.168.0.100

2. How to set on the host PC (generic steps):
    (1) Open the operating system network settings and identify the Ethernet adapter that is physically connected to 
    the board.
    (2) Edit that adapter’s IPv4 settings and configure the following:
        IP address: 192.168.0.1
        Subnet mask: 255.255.255.0
        Default gateway: leave empty (not required for local ping test)

3. This EP requires only one Ethernet cable for verification.
   The same host PC Ethernet interface can be connected to ETH0, ETH1, and ETH2 one at a time to verify link-up and 
   basic connectivity on each board port. The application monitors 3 Ethernet ports and reports link status changes on 
   the console.

4. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10186100
 
5. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called    
   _SEGGER_RTT) in .map file generated in the build configuration folder (Debug/Release).