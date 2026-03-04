/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CAN-FD running on Renesas RZ T2M using channel 0 and channel 1 on board. 
    When pressing any key on the RTT Viewer, data is transmitted from one channel to the other.
    On 1st transmission, Channel 0 transmits data to Channel 1. Channel 1 displays the received data.
    On 2nd transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data.
    then Channel 0 changes CAN frame to CANFD frame and updates data
    On 3rd transmission, Channel 0 transmits updated data to Channel 1. Channel 1 displays the received data.
    then Channel 1 changes CAN frame to CANFD frame and updates data to transmits back to Channel 0 as ACK.
    On 4th transmission, Channel 1 transmits updated data to Channel 0 as ACK. Channel 0 displays the received data

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x TJA1050 CAN Bus Transceiver Module.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot Mode :External 16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:
        1. *External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. *Set SW6 following.
        SW6.1  : OFF
        SW6.9  : OFF
        SW6.10 : ON

        3. Connect CAN network 

        |   CN1 header 20 way  |      TJA1050 CAN Bus     |
        |----------------------|--------------------------|
        |       CN1.3     <----|---->   EXTERNAL CAN TX   |
        |       CN1.6     <----|---->   EXTERNAL CAN RX   |
        |       CN1.10    <----|---->   GND               |
        |       CN1.20    <----|---->   5V                |
        |----------------------|--------------------------|

        |         J33          |      TJA1050 CAN Bus     |
        |----------------------|--------------------------|
        |       J33.1     <----|---->   CAN_H             |
        |       J33.2     <----|---->   CAN_L             |
        |----------------------|--------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).