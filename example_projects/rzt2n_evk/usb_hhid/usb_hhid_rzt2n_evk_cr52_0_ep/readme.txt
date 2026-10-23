File Name   : readme.txt
Description : Readme of the RZ/T2N USB HHID (Host HID) example project
              (usb_hhid_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the USB HHID (Host Human Interface Device) function of
    the RZ/T2N on the Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program initializes the SCI0
    and USB. If you connect a Keyboard or Mouse device to the Evaluation board (J2), the HHID
    application receives the Key code or Mouse data from the HID device and outputs the data on
    the terminal.

    The details of the operation flow are shown as below.

      1. Start the debugging and sample program as hal_entry().

      2. R_SCI_UART_Open() initializes SCI0 as UART settings.

      3. R_USB_Open() initializes the USB settings.

      4. R_USB_EventGet() is executed and gets the USB event.

      5. Program processing changes at USB events.

           Case : USB_STATUS_CONFIGURED
                  1. R_USB_HHID_TypeGet() is executed and gets HID protocol.
                     (USB Mouse/USB Keyboard/Other Type.)
                  2. R_USB_HHID_MaxPacketSizeGet() is executed and gets max packet size for the
                     connected HID device.
                  3. set_protocol() is executed and send the HID request (SetProtocol) to HID
                     device.

           Case : USB_STATUS_READ_COMPLETE
                  1. R_USB_Read() is executed and receives the Key code or Mouse data from the
                     HID device.
                  2. output_log() is executed and outputs the data on the terminal.

           Case : USB_STATUS_REQUEST_COMPLETE
                  1. If the USB event setup request type is USB_HID_SET_PROTOCOL,
                     R_USB_Read() is executed and receives data from the HID device.
                  2. output_log() is executed and outputs the data on the terminal.

      After that, repeat steps 4 and 5.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GNU ARM Embedded (Version:13.3.1.arm-13-24)
      RZ FSP version   : 4.2.0

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board  x 1
    - Target Core  : Cortex-R52 CPU0
    - Boot mode    : RAM execution

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A
    - 1 x USB HID device (a USB keyboard or a USB mouse)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - Use USB Host (J2)
        SW11.1  : ON
        SW11.2  : OFF


5. Hardware Connections:

    Connect the Evaluation board and the PC as below.

    - J31 : IAR I-jet                                  (for debugging in IAR EWARM)
    - J32 : USB Micro-B cable to PC                    (for debugging in e²studio, J-Link OB)
    - J33 : USB Micro-B cable to PC                    (for serial communication, COM port)
    - J7  : USB Type-C cable with the AC adapter       (for power supply)

    Note that J31 is used in the IAR EWARM environment and J32 is used in the e²studio
    environment. Connect the one for the environment to be used.

    - Debugging Function Selection
      J29 : Open  (for debugging in e²studio, J-Link OB)
      J29 : Short (for debugging in IAR EWARM)

    Connect the USB HID device (a USB keyboard or a USB mouse) to J2 (USB Host) of the
    Evaluation board.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - This sample program displays the usb_hhid test results on a PC (terminal software) via a
      COM port. Set the configuration of Terminal Emulator (Tera Term) as below.

        Port         : COM port provided by the RZ board.
                       (When connected to a PC via a USB serial port, you will see two COM ports.
                        Connect to the higher numbered COM port of the two ports.
                        If no characters are output to the terminal when the program is running,
                        try connecting the other COM.)
        Speed        : 115200 bps
        Data         : 8 bit
        Parity       : none
        Stop bits    : 1 bit
        Flow control : none

    - For more information about the data received from HID devices, please refer to the
      following USB convention documents.

        * HID Usage Tables FOR Universal Serial Bus (USB) Version 1.4
               PDF Name   : hut1_4.pdf
               URL        : https://www.usb.org/document-library/hid-usage-tables-14
               Reference  : Page.89 "10 Keyboard/Keypad Page"
                            (Contains a Key code table)

        * Universal Serial Bus (USB) Device Class Definition for Human Interface Devices (HID)
          Version 1.11
               PDF Name   : hid1_11.pdf
               URL        : https://usb.org/document-library/device-class-definition-hid-111
               Reference  : Page.59 "B.1 Protocol 1 (Keyboard)"
                            Page.61 "B.2 Protocol 2 (Mouse)"
                            (Contains a description of the HID data register)

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "usb_hhid_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[usb_hhid_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "usb_hhid_rzt2n_evk_cr52_0_ep\usb_hhid_rzt2n_evk_cr52_0_ep.eww" in the folder
     unzipped in "2.".

  5. Start the Smart Configurator from [Tools].
     The Smart Configurator must be registered in advance under [Tools]-[Configure Tools].

      Menu Text          : "FSP Smart Configurator"
      Command            : "$RASC_EXE_PATH$"
      Argument           : "--compiler IAR configuration.xml"
      Initial Directory  : "$PROJ_DIR$"

    - Specify the absolute installation path of [rasc.exe] in "RASC_EXE_PATH" in [buildinfo.ipcf].

  6. Click [Generate Project Content] to generate the code in the Smart Configurator.

  7. Select [Project]-[Rebuild All] from the EWARM menu.
     - During the build, if a window appears searching for Smart Configurator, enter the number
       that indicates fsp v4.2.0 and press Enter.

  8. While the board and I-jet are connected, click on the [Download and debug] button in the
     "Project" toolbar.
     The program will break at the first code in "main".

  9. Press the [Go] button again to execute the program.
     Initial settings of cores and registers in SSLB and the sample application program will be run.


  - e²studio environment

  1. Connect USB Micro-B to J32 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "usb_hhid_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[usb_hhid_rzt2n_evk_cr52_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.