# Introduction
 
This example project demonstrates Ethernet functionality using the GMAC driver on RZ/T2L based on Renesas FSP.
The program initializes Ethernet port2 (ETH2) with predefined MAC and IP addresses and waits for communication from a host PC.
Once initialized, the board listens for ARP and ICMP echo requests from a host PC connected to Ethernet port 2.
When an ARP request is received, the board responds with its MAC address.
When a ping request is received, the board sends an ICMP echo reply back to the sender.
This allows users to verify Ethernet communication from the host PC using basic ping commands.

Please refer to the Example Project Usage Guide for general information on example projects and [readme.txt](./readme.txt) for specifics of operation.

## Required Resources
To build and run the Ethernet example project, the following resources are needed.

### Hardware
* 1x Renesas Starter Kit+ for RZ/T2L
* 1x Ethernet Cable

Refer to [readme.txt](./readme.txt) for information on how to connect the hardware.

### Software
1. Refer to the software required section in Example Project Usage Guide

## Related Collateral References
The following documents can be referred to for enhancing your understanding of
the operation of this example project:
- [FSP User Manual on GitHub](https://renesas.github.io/rz-fsp/)

# Project Notes

## FSP Modules Used
List of important modules that are used in this example project. Refer to the FSP User Manual for further details on each module listed below.

| Module Name | Usage | Searchable Keyword  |
|-------------|-----------------------------------------------|-----------------------------------------------|
|Ethernet | Driver for the Ethernet peripheral to demonstrate ARP and ICMP Echo Response (Ping) functionalities using multiple GMAC interfaces. | gmac|

The table below lists the FSP provided API used at the application layer by this example project.

| API Name    | Usage                                                                          |
|-------------|--------------------------------------------------------------------------------|
| R_GMAC_Open | This API is used to initialize the Ethernet peripheral, including ETHERC, EDMAC, and PHY, and start PHY auto-negotiation to establish a link. |
| R_GMAC_Write | This API is used to transmit Ethernet frame. Transmits data from the location specified by the pointer to the transmit buffer, with the data size equal to the specified frame length. |
| R_GMAC_Read | This API is used to receive an Ethernet frame by receiving data to the location specified by the pointer to the receive buffer. |
| R_GMAC_Close | This API is used to close the Ethernet channel by disabling interrupts, releasing hardware locks, and powering down the peripheral. |
| R_GMAC_LinkProcess | This API is used to handle link up/down status and magic packet detection by executing necessary PHY interface processing.  |

## Verifying operation
1. Import, generate and build Ethernet EP in e2studio.
   Before running the example project, make sure hardware connections are done.
2. Download Ethernet EP to one Renesas RZ MPU Evaluation kit and run the project.
3. On the host PC, configure Ethernet port with a static IP address in the same subnet as the board's default IP (e.g., 192.168.10.1).
4. Now open Jlink RTT Viewer and connect to RZ MPU board.
5. User can observe the link-up status of all Ethernet ports via messages displayed on J-Link RTT Viewer.
6. Also user can open the command prompt and perform ping operations to each board IP address (192.168.10.100).
7. The board responds to ARP and ICMP Echo requests, allowing users to verify Ethernet functionality by checking successful ping replies.

   Below images showcases the Ethernet output on JLinkRTT_Viewer and Command Prompt:

   <img src="images/Ethernet_Banner.png" alt="Ethernet_Banner" title="Banner Info" width="800"/>

   <img src="images/Ethernet_Ping_Port.png" alt="Ethernet_Ping_Port" title="CMD output - Ethernet_Ping_3_Ports" width="800"/>

   <img src="images/RTT_Ethernet_Ping_Result.png" alt="RTT_Ethernet_Ping_Result" title="RTT output - RTT_Ethernet_Ping_Result" width="800"/>
