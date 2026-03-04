/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This software is a sample program designed to demonstrate ping response functionality using the Ethernet driver for the RZ/N2H device.
    By default, it enables Ethernet communication via three ports: ETH0, ETH1, and ETH2 on the RZ/N2H board.
    To evaluate simultaneous 3-port Ethernet communication, the host PC must have three available Ethernet interfaces, 
    each connected to a corresponding port on the board.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

        1.   *Configuration of Evaluation board by Switches and Jumper Blocks
        The operating mode of the RZ/N2H
        DSW3.1   : ON
        DSW3.2   : OFF
        DSW3.3   : ON

        2. *CPU0 ATCM wait cycle = 1 wait cycle
        DSW3.4   : OFF

        3. *Set Connectors following.
        CN6:2-3 short
        CN7:2-3 short
        CN8:1-2 short
  
        4. *Configuring the board circuit
        DSW5.6: ON
        DSW5.7: ON
        DSW5-8: OFF

        DSW8.1: OFF
        DSW8.2: ON

        DSW12.3: OFF
        DSW12.4: ON 

        DSW13.1: ON
        DSW13.2: OFF
        DSW13.3: ON
        DSW13.4: OFF
        DSW13.5: ON
        DSW13.6: OFF
        DSW13.7: OFF
        DSW13.8: ON

        DSW14.3: ON
        DSW14.4: OFF
        DSW14.5: ON
        DSW14.6: OFF

        DSW21.1: ON
        DSW21.2: OFF
        DSW21.3: ON 

        5. Connect ETH0, ETH1, and ETH2 ports on the board to three separate Ethernet ports on the host PC using Ethernet cables.

    Configuring the Evaluation board circuit:
        - USB connection between Evaluation board and PC
          CN26 - USB Type-A to micro USB cable.

Note:
1. The host PC must have three Ethernet interfaces configured with static IPs on the same subnet as the board:
     ETH0 <-> 192.168.10.1 (PC side), board = 192.168.10.100
     ETH1 <-> 192.168.20.1 (PC side), board = 192.168.20.100
     ETH2 <-> 192.168.30.1 (PC side), board = 192.168.30.100

2. How to set on the host PC (generic steps):
    (1) Open the operating system’ network settings and identify the Ethernet adapter that is physically connected to   
        ETH0 on the board.
    (2) Edit that adapter’s IPv4 settings and configure the following:
        IP address: 192.168.10.1
        Subnet mask: 255.255.255.0
        Default gateway: leave empty (not required for local ping test)
    (3) Repeat the above steps for the adapters connected to ETH1 and ETH2, using IP addresses 192.168.20.1 and 192.168.
        30.1 respectively.

3. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x10188100
 
4. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).