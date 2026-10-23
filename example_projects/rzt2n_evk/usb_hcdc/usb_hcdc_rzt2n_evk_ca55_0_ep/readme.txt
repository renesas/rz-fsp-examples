File Name   : readme.txt
Description : Readme of the RZ/T2N USB HCDC (Host CDC) example project
              (usb_hcdc_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project demonstrates the USB HCDC (Host Communications Device Class) function
    of the RZ/T2N on the Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program establishes a USB
    connection and goes into a connection standby state with the PCDC device.

    Two Evaluation boards are required. This project runs on the host side board, and the USB
    PCDC example project runs on the counterpart board as the PCDC device.

    The details of the operation flow are shown below.

      1. After debugging the connection to Evaluation board,
         this sample program establishes a USB connection and goes into a connection standby
         state with the PCDC device.
         The other hands, as the PCDC device, connects to evaluation board that PCDC sample
         program was executed.

      2. The HCDC device sends 512 bytes data to the PCDC device.

      3. The PCDC device returns the received character data to the HCDC device.

      4. HCDC device sends the data returned from the PCDC device again.

      After that, the application repeats steps 3 and 4.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0


    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board x 2
                     ( board(1) : USB HCDC, the host side (this project)
                       board(2) : USB PCDC, the peripheral side )
    - Target Core  : Cortex-A55 Core0
    - Boot mode    : RAM execution

    * Prepare the following for EACH board (2 sets in total).
    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A

    * Prepare the following to connect the two boards.
    - 1 x USB Micro-B cable (for USB communication between the host board and the peripheral
      board)


4. Hardware Configurations:

    Configure this host side Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - Use USB Host (J2)
        SW11.1  : ON
        SW11.2  : OFF

    SW11.1 and SW11.2 select the USB host function of J2. The peripheral side board uses a
    different setting; refer to the readme.txt of the USB PCDC example project for it.


5. Hardware Connections:

    Connect each Evaluation board and the PC as below.

    - J31 : IAR I-jet                                  (for debugging in IAR EWARM)
    - J32 : USB Micro-B cable to PC                    (for debugging in e²studio, J-Link OB)
    - J33 : USB Micro-B cable to PC                    (for serial communication, COM port)
    - J7  : USB Type-C cable with the AC adapter       (for power supply)

    Note that J31 is used in the IAR EWARM environment and J32 is used in the e²studio
    environment. Connect the one for the environment to be used.
    Each board needs its own power supply through its own J7.

    - Debugging Function Selection
      J29 : Open  (for debugging in e²studio, J-Link OB)
      J29 : Short (for debugging in IAR EWARM)

    Connect J2 (USB Host) of this host side board to the USB device connector of the peripheral
    side board with the USB Micro-B cable.

        RZ/T2N board(1)                      RZ/T2N board(2)
        USB HCDC (host)                      USB PCDC (peripheral)

        J2 (USB Host) O===== USB Micro-B cable =====O USB device connector

    Refer to the readme.txt of the USB PCDC example project for the connector of the peripheral
    side board.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply of both boards.


Note:

    - The HCDC device must be free-run before running the PCDC device.
      Start this host side project first and let it run. It goes into a connection standby
      state and waits for the PCDC device, which is the expected state. After that, start the
      peripheral side project.

    - This sample program displays the startup log on a PC (terminal software) via a
      COM port. Set the configuration of Terminal Emulator (Tera Term) as below.

        Port         : COM port provided by the RZ board.
                       (When connected to a PC via a USB serial port, you will see two COM ports.
                        Connect to the smaller numbered COM port of the two ports.
                        If no characters are output to the terminal when the program is running,
                        try connecting the other COM.)
        Speed        : 115200 bps
        Data         : 8 bit
        Parity       : none
        Stop bits    : 1 bit
        Flow control : none

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "usb_hcdc_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  Note (two boards are used at the same time)
  - Two debug sessions are required, one for this host project and one for the peripheral
    project. Start the IDE twice, and use a different workspace for each instance.
  - Start the debug session of this USB HCDC (host) side first and let it free-run, and then
    start the USB PCDC (peripheral) side.
  - When the two boards are connected to the same PC, two emulators are recognized. Select the
    emulator of the target board before starting the debug session.

  The recommended order of the whole procedure is as below.

    1. Turn off the power of both boards.
    2. Connect J2 of the host board to the USB device connector of the peripheral board.
    3. Check SW11.1 and SW11.2 on this host board.
    4. Turn on the power of both boards.
    5. Start debugging this host project and resume it. It waits for the PCDC device.
    6. Start debugging the peripheral project and resume it.
    7. The host sends 512 bytes of data to the peripheral.
    8. The peripheral returns the received data, and the host sends it again.


  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[usb_hcdc_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "usb_hcdc_rzt2n_evk_ca55_0_ep\usb_hcdc_rzt2n_evk_ca55_0_ep.eww" in the folder
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
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "usb_hcdc_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[usb_hcdc_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.