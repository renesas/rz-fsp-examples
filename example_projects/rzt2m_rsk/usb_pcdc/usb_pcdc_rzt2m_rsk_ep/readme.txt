/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example project demonstrates the functionality of usb_pcdc driver on Renesas RZ MPUs using RZ/T2M boards.
    This project demonstrates basic serial communication between a host PC and the target board using Tera Term. After launching Tera Term, the user establishes a new serial connection. Once the connection is established, any key pressed on the host PC is transmitted via the USB serial interface to the target device. The corresponding character data is then displayed on RTT Viewer.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB 2.0 to USB Mini cable.
    1 x PC with at least 2 USB port(1 for debug and 1 for com port).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot Mode :External 16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:

        1. *External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. *Set SW6 following.
        SW6.1 : ON

        4. Connect CN11 USBF port under RSK to HOST PC

5. Software settings :
    - Execute TeraTerm, and click File -> New connection.
    - Select the "Serial" and Choose Port:COMx: USB Serial Device(COMx).
    - Click Setup -> Serial port, and set parameters below:
       Port           :COMx
       Baud rate      :9600
       Data           :8bit
       Parity         :none
       Stop           :1bit
       Flow Control   :none
       Terminal delay :0 msec/char  /  0 msec/line   
    
Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
