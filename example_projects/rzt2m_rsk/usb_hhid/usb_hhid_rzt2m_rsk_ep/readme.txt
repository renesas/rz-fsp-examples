/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates basic functionalities of USB HHID driver on Renesas RZ 
    MPUs based on Renesas FSP. USB HHID driver configures keyboard as a hhid device. on pressing key from
    the keyboard, received keycode is decoded and displayed on the Jlink RTTViewer.
    Error and info messages will be printed on JlinkRTTViewer.

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
    1 x PC Keyboard.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot Mode :External 16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:
        1. External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5  : OFF

        3. Connect keyboard 
    Connect USB keyboard to CN10 connector

Note:
1. Enter any key — a–z, A–Z, 0–9, or special characters. For more information about the data received from HID devices, please refer to the following USB convention documents.
        * HID Usage Tables FOR Universal Serial Bus (USB) Version 1.4
            PDF Name   : hut1_7.pdf
            URL        : https://www.usb.org/document-library/hid-usage-tables-17
            Reference  : Page.89 "10 Keyboard/Keypad Page"
                           (Contains a Key code table)

        * Universal Serial Bus (USB) Device Class Definition for Human Interface Devices (HID) Version 1.11
            PDF Name   : hid1_11.pdf
            URL        : https://usb.org/document-library/device-class-definition-hid-111
            Reference  : Page.59 "B.1 Protocol 1 (Keyboard)"
                         Page.61 "B.2 Protocol 2 (Mouse)"
                           (Contains a description of the HID data register)
            Document versions and URLs above may be updated/changed over time; please refer to the latest versions on the USB-IF website.

2. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

3. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).