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
    For each test, verify both the host-side result and the corresponding logs on the Tera Term.
    Link up/down status for each Ethernet port is also monitored and printed on the console.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.
    1 x Ethernet cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        - The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
          SW14.1 : ON
          SW14.2 : OFF
          SW14.3 : ON
          SW14.6 : OFF

        - CPU0 ATCM wait cycle = 1 wait cycle
          SW14.4 : OFF

        - Use UART data for USB-to-serial conversion
          SW4.5  : ON
          SW4.6  : OFF
          SW4.7  : ON
          SW4.8  : OFF

        - Use Ethernet ports
          SW2.6 : ON
          SW2.7 : ON
          SW4.1 : ON
          SW4.2 : OFF
          SW4.3 : ON
          SW4.4 : OFF
          SW6.2 : OFF
          SW6.3 : ON
          SW6.4 : OFF
          SW6.5 : ON
          CN9   : 1-2 short
          CN37  : 2-3 short
          CN38  : 2-3 short
          CN39  : 1-2 short

5. Hardware Connections:
    (1) Connect the CN14 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN34 port on evaluation board to the PC using USB Mini-B cable for serial terminal 
        communication (Tera Term).
    (3) Connect the CN46 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (4) Slide SW16 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
    (5) Connect ETH port on the board to Ethernet port on the host PC using Ethernet cable.

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

2. The host PC must have one Ethernet interface configured with static IP on the same subnet as the board:
   ETH <-> 192.168.0.1 (PC side), board = 192.168.0.100

3. How to set on the host PC (generic steps):
    (1) Open the operating system network settings and identify the Ethernet adapter that is
    physically connected to the board.

    (2) Edit that adapter’s IPv4 settings and configure the following:
        IP address: 192.168.0.1
        Subnet mask: 255.255.255.0
        Default gateway: leave empty (not required for local ping test)

4. This EP requires only one Ethernet cable for verification.
   The same host PC Ethernet interface can be connected to ETH0, ETH1, and ETH2 one at a time to verify link-up and
   basic connectivity on each board port. The application monitors 3 Ethernet ports and reports link status changes
   on the console.