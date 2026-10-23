File Name   : readme.txt
Description : Readme of the RZ/T2N CGC example project (cgc_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    After debug connection to Evaluation board, this sample program initializes CGC function.
    The LED blinks with the software wait.
    CGC changes the CPU operating clock and changes the LED blinking interval.

    The details of the operation flow are shown below.

    1. Start the debugging and sample program as hal_entry().

    2. R_CGC_Open() initialize CGC settings.

    3. R_CGC_ClocksCfg() is executed, and switch to the "slowest clock" setting.

    4. Blink the LED10(USER_LED2) at slowest clock.

    5. R_CGC_ClocksCfg() is executed, and switch to the "fastest clock" setting.

    6. Blink the LED10(USER_LED2) at fastest clock.

    The main files created for the sample program are as below

     - r_cgc_api.h:
       The file containing CGC interface.

     - r_cgc.h:
       The file containing CGC instances.

     - r_cgc.c:
       The file containing CGC driver.

     - r_cgc_cfg.h:
       The file containing configuration header for CGC.

     - hal_data.h, hal_data.c:
       The file containing configuration parameter for CGC.

     - hal_entry.c:
       The file containing sample application of CGC.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version : 4.2.0

    - Terminal Emulator (Tera Term)

3. Hardware Requirements:

    - Target board
      : RZ/T2N Evaluation board

    - Target Core
      : Cortex-A55 Core0

    - Boot mode
      : RAM execution

    - Miscellaneous
      : USB Micro-B cable (for J-Link OB)
      : USB Micro-B cable (for serial communication)
      : IAR I-jet (for debugging in IAR EWARM)
      : USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
      : AC adapter for USB supporting 15V/3A


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
      SW8.1 : ON
      SW8.2 : OFF
      SW8.3 : ON

    - 2nd Boot CPU : Cortex-A55 Core0
      SW10.10 : OFF
      SW10.7  : OFF


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

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn
    on the power supply.


Note:

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

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking "cgc_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by [Index] > [Rebuild].

    - Before starting debug, please push reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button during debugging.


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[cgc_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu,
     and open "cgc_rzt2n_evk_ca55_0_ep\cgc_rzt2n_evk_ca55_0_ep.eww" in the folder unzipped in "2.".

  5. Start the Smart Configurator from [Tools].
     The Smart Configurator must be registered in advance under [Tools]-[Configure Tools].

      Menu Text          : "FSP Smart Configurator"
      Command            : "$RASC_EXE_PATH$"
      Argument           : "--compiler IAR configuration.xml"
      Initial Directory  : "$PROJ_DIR$"

    - Specify the absolute installation path of [rasc.exe] in "RASC_EXE_PATH" in [buildinfo.ipcf].

  6. Click [Generate Project Content] to generate the code in the Smart Configurator.

  7. Select [Project]-[Rebuild All] from the EWARM menu.
  Note:
    - During the build, if a window appears searching for Smart Configurator, enter the number that indicates fsp v4.2.0 and press Enter.

  8. While the board and I-jet are connected, click on the [Download and debug] button in the "Project" toolbar.
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
    - Check the [select archive file] and select "cgc_rzt2n_evk_ca55_0_ep.zip", and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select [Debug As]->[cgc_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.

