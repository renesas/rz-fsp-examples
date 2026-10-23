
File Name   : readme.txt
Description : Readme of the RZ/T2N USB PCDC example project
              (usb_pcdc_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates USB Peripheral Communications Device Class (PCDC)
    operation on the Renesas RZ/T2N Evaluation board.

    After starting the sample application, connect the USB Function port (J3) of the Evaluation
    board to the host PC. The board is recognized as a USB Serial Device. Character data sent
    from the host PC is displayed on the terminal software.

    The details of the operation flow are shown below.
      1. Start debugging and execute the sample application.
      2. Connect the J3 USB Function port on the Evaluation board to the host PC.
      3. Start Tera Term and select [File] -> [New connection].
      4. Select [Serial] and choose "COMx: USB Serial Device (COMx)".
      5. Configure the serial port as described in the Note section.
      6. Push any key on the host PC.
      7. The board echoes back the received characters. Check the echo in the separate
         Tera Term window opened for the USB Serial Device COM port in steps 3 and 4,
         not in the window of the J33 COM port that displays the startup banner.


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
    - 1 x USB Micro-B cable (for USB communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - Use USB Function (J3)
        SW11.5  : ON
        SW11.6  : OFF
        SW11.7  : OFF


5. Hardware Connections:

    Connect the Evaluation board and the PC as below.

    - J31 : IAR I-jet                                  (for debugging in IAR EWARM)
    - J32 : USB Micro-B cable to PC                    (for debugging in e²studio, J-Link OB)
    - J3  : USB Micro-B cable to PC                    (for USB communication)
    - J7  : USB Type-C cable with the AC adapter       (for power supply)

    Note that J31 is used in the IAR EWARM environment and J32 is used in the e²studio
    environment. Connect the one for the environment to be used.

    - Debugging Function Selection
      J29 : Open  (for debugging in e²studio, J-Link OB)
      J29 : Short (for debugging in IAR EWARM)

    After all the connections required for debugging are completed, switch SW1 "POWER_SW"
    from OFF to ON to turn on the power supply.

    After starting debugging, connect the J3 USB Function port to the host PC.


Note:

    - The startup banner and Project Overview are output through the board serial connector
      J33 at 115200 bps, 8 data bits, no parity, 1 stop bit, and no flow control. This board
      serial output is separate from the USB CDC COM port through J3 described below.
      Connect J33 to the PC with a USB Micro-B cable and open the higher numbered board
      COM port in Tera Term. If no characters are displayed, try the other board COM port.

    - Set the configuration of Terminal Emulator (Tera Term) as below.

        Port           : COMx: USB Serial Device (COMx)
        Baud rate      : 9600
        Data           : 8 bit
        Parity         : none
        Stop bits      : 1 bit
        Flow control   : none
        Terminal delay : 0 msec/char / 0 msec/line

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "usb_pcdc_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[usb_pcdc_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu,
     and open "usb_pcdc_rzt2n_evk_cr52_0_ep\usb_pcdc_rzt2n_evk_cr52_0_ep.eww" in the folder unzipped in "2.".

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

  10. Connect the J3 USB Function port to the host PC and check the sample operation using
      Tera Term as described in the Note section.


  - e²studio environment

  1. Connect USB Micro-B to J32 and USB Type-C cable with AC adapter to J7.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "usb_pcdc_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[usb_pcdc_rzt2n_evk_cr52_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.

  9. Connect the J3 USB Function port to the host PC and check the sample operation using
     Tera Term as described in the Note section.
