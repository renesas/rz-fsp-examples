/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project shows the operation of CAN-FD running on Renesas RZ MPUs using channel 0 and channel 1 on board.
    On pressing any key on the RTT Viewer, data is transmitted from one channel to other.
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
    RZ supported boards: RZ/V2N-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    2 x CAN transceiver modules.
4. Hardware Configurations:
    (1) Boot Mode:
        CA55 coldboot (SCIF boot).
    (2) Set for DIP switches and jumpers as follow.
        - DSW1 : 1-ON,2-OFF,3-ON,4-OFF,5-ON,6-OFF,7-ON,8-OFF.
        - Others : Default.

5. Hardware Connections:
    (1) Connect the CN13 on the RZ/V2N Evaluation Board to a USB PD charger using a USB Type-C cable 
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with JTAG cable.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect PMOD LEDs to Pmod Type3/3A(CN3) (pin 1-6) on RZ/V2N EVK Expansion Board.
    (4) Connect RZ/V2N EVK Expansion Board and 2 CAN transceiver modules:
            Signal connections:
            +-------------------------------+-------------------------+-------------------------+
            | RZ/V2N EVK Expansion Board    | CAN Transceiver 1       | CAN Transceiver 2       |
            +-------------------------------+-------------------------+-------------------------+
            | Pmod Type1/1A(CN1) PIN1       | Tx Pin                  |                         |
            +-------------------------------+-------------------------+-------------------------+
            | Pmod Type1/1A(CN1) PIN2       | Rx Pin                  |                         |
            +-------------------------------+-------------------------+-------------------------+
            | Pmod Type1/1A(CN1) PIN3       |                         | Tx Pin                  |
            +-------------------------------+-------------------------+-------------------------+
            | Pmod Type1/1A(CN1) PIN4       |                         | Rx Pin                  |
            +-------------------------------+-------------------------+-------------------------+

            Power connections:
            +-------------------------------+-------------------------+-------------------------+
            | RZ/V2N EVK Expansion Board    | CAN Transceiver 1       | CAN Transceiver 2       |
            +-------------------------------+-------------------------+-------------------------+
            | H3 hole (VCC on board)        | Pin VCC                 | Pin VCC                 |
            +-------------------------------+-------------------------+-------------------------+
            | Pmod Type1/1A(CN1)            | Pin GND                 | Pin GND                 |
            | PIN11 (GND on board)          |                         |                         |
            +-------------------------------+-------------------------+-------------------------+

            CAN Bus connections (between 2 CAN transceiver modules):
            +-------------------------+-------------------------+
            | CAN Transceiver 1       | CAN Transceiver 2       |
            +-------------------------+-------------------------+
            | Can_H Pin               | Can_H Pin               |
            +-------------------------+-------------------------+
            | Can_L Pin               | Can_L Pin               |
            +-------------------------+-------------------------+
    
Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x0800bbd0
2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
3. The user must enter data of no more than 15 bytes.
4. PMOD LED behavior as below:
    a. When CAN transmission operation is in progress:
       LEDs will be turned ON from LD0 to LD3 then turned OFF sequentially from LD0 to LD3.
    b. When CAN transmission operation is successful:
       LEDs LD0 through LD3 toggle continuously.
    c. When any error occurs in CAN operation, all LEDs will be turned ON.