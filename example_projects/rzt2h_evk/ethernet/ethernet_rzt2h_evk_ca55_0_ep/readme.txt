/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This software is a sample program designed to demonstrate ping response functionality using the Ethernet driver for the RZ/T2H device.
    By default, it enables Ethernet communication via three ports: ETH0, ETH1, and ETH2 on the RZ/T2H board.
    To evaluate simultaneous 3-port Ethernet communication, the host PC must have three available Ethernet interfaces, 
    each connected to a corresponding port on the board.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.
    3 x Ethernet cables.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

        1. *Configuration of RZ/T2H Evaluation board by Switches and Jumper Blocks
        The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
        SW14.1 : ON
        SW14.2 : OFF
        SW14.3 : ON
        SW14.6 : OFF

        2. *CPU0 ATCM wait cycle = 1 wait cycle
        SW14.4 : OFF

        3. *Set Connectors following.
        CN9:1-2 short
        CN37:2-3 short
        CN38:2-3 short
        CN39:1-2 short
  
        4. *Configuring the board circuit
        SW2.6:ON
        SW2.7:ON

        SW4.1:ON
        SW4.2:OFF
        SW4.3:ON
        SW4.4:OFF

        SW6.2:OFF
        SW6.3:ON
        SW6.4:OFF
        SW6.5:ON

        5. *Use UART data for USB-to-serial conversion
        SW4.5:ON
        SW4.6:OFF
        SW4.7:ON
        SW4.8:OFF

        5. Connect ETH0, ETH1, and ETH2 ports on the board to three separate Ethernet ports on the host PC using Ethernet cables.

    Configuring the Evaluation board circuit
        - USB connection between Evaluation board and PC
          CN34 - USB Type-A to USB Mini-B cable
          CN14 - USB Type-A to micro USB cable

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