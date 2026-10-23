File Name   : readme.txt
Description : Readme of the RZ/T2N FreeRTOS+TCP example project (FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This sample program is a FreeRTOS+TCP sample that uses the RZ/T2N Ethernet driver.

    In the default state, this sample program performs Ethernet communication on the three
    ports ETH0, ETH1 and ETH2. GMAC0 is connected to ETH0, ETH1 and ETH2 via the ETHSW.

    Note:
      - When evaluating 3-port simultaneous communication, three Ethernet ports are required
        on the host PC side.

      - This sample program does not support Ethernet communication on ETH3 and ETH4.

      - GMAC0 is the only GMAC supported by this sample program.
        (GMAC1 and GMAC2 are not supported.)

      - On the RZ/T2N Evaluation board, ETH0, ETH1 and ETH2 can only be used in RGMII mode.

      - The board is assigned the IP address "192.168.0.100".
        Set the IP address of the host PC Ethernet port so that it belongs to the same network.

    The details of the operation flow are shown as below.

    1. Start the debugging and sample program as hal_entry().

    2. The network thread initializes the Ethernet driver and waits for link up.

    3. Enter the "ping 192.168.0.100" command from the host PC command prompt,
       and confirm that there is a ping response.

    4. Confirm that "LED0" on the board blinks at a 1 second cycle.

    5. Send a UDP packet from the host PC to IP address "192.168.0.100" and port number
       "10000", and confirm that there is an echo response.

    6. Send a TCP packet from the host PC to IP address "192.168.0.100" and port number
       "10000", and confirm that there is an echo response.

    The main files created for the sample program are as below

     - hal_entry.c:
       The file containing the entry point of the sample application.

     - net_thread_entry.c:
       The file containing the network initialization and link management thread.

     - tcp_echo_server_thread_entry.c:
       The file containing the TCP echo server thread.

     - udp_echo_server_thread_entry.c:
       The file containing the UDP echo server thread.

     - blinky_thread_entry.c:
       The file containing the LED blink thread.

     - syscalls.c:
       The file containing the system call stubs for the standard library.

     - hal_data.h, hal_data.c:
       The file containing configuration parameter for the peripherals used.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain : GNU ARM Embedded (Version:13.3.1.arm-13-24)
      RZ FSP version : 4.2.0
      J-Link : v9.44

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board
      : RZ/T2N Evaluation board

    - Target Core
      : Cortex-R52 CPU0

    - Operating frequency
      : 800MHz

    - Boot mode
      : RAM execution

    - Miscellaneous
      : USB Micro-B cable (for J-Link OB)
      : USB Micro-B cable (for serial communication)
      : Ethernet cable x3 (for Ethernet communication)
      : IAR I-jet (for debugging in IAR EWARM)
      : USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
      : AC adapter for USB supporting 15V/3A


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
      SW8.1 : ON
      SW8.2 : OFF
      SW8.3 : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
      SW10.10 : ON
      SW10.7  : OFF

    - Use Ethernet
      SW7.2  : ON
      SW10.2 : ON
      SW12.3 : ON
      SW12.4 : ON
      J15    : 1-2 short
      J41    : 2-3 short


5. Hardware Connections:

    Connect the Evaluation board and the PC as below.

    - J31 : IAR I-jet                                  (for debugging in IAR EWARM)
    - J32 : USB Micro-B cable to PC                    (for debugging in e²studio, J-Link OB)
    - J33 : USB Micro-B cable to PC                    (for serial communication, COM port)
    - J7  : USB Type-C cable with the AC adapter       (for power supply)

    Additional connections for this sample program:

    - USB connection between Evaluation board and PC
      J32 - USB Micro-B

    - Ethernet connection between Evaluation board and host PC
      ETH0, ETH1, ETH2 - RJ45

    Note that J31 is used in the IAR EWARM environment and J32 is used in the e²studio
    environment. Connect the one for the environment to be used.

    - Debugging Function Selection
      J29 : Open  (for debugging in e²studio, J-Link OB)
      J29 : Short (for debugging in IAR EWARM)

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn
    on the power supply.


Note:

    - This sample program displays the startup log and network settings on a PC (terminal software) via a
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

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking "FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by [Index] > [Rebuild].

    - Before starting debug, please push reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button during debugging.


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect the Ethernet cable to the host PC Ethernet port and to the "ETH0",
     "ETH1" or "ETH2" connector on the board.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu,
     and open "FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep\FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep.eww"
     in the folder unzipped in "2.".

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
     Also, connect the Ethernet cable to the host PC Ethernet port and to the "ETH0",
     "ETH1" or "ETH2" connector on the board.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep.zip", and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select [Debug As]->[FreeRTOSplusTCP_rzt2n_evk_cr52_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.
