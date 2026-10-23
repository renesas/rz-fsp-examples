File Name   : readme.txt
Description : Readme of the RZ/T2N Ethernet (ETHSW) example project
              (ethernet_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project is a sample program that performs a ping response using the Ethernet
    driver for the RZ/T2N, on the Renesas RZ/T2N Evaluation board.

    This sample software performs Ethernet communication with 3 ports ETH0, ETH1 and ETH2 in
    the default state. GMAC0 is connected to ETH0, ETH1 and ETH2 via the ETHSW (Ethernet
    Switch). When evaluating 3-port simultaneous communication, 3 Ethernet ports are required
    on the HOST PC side.

    Limitations of this sample software.
      - ETH3 and ETH4 are not supported.
      - GMAC0 is the only GMAC that is supported. (GMAC1 and GMAC2 are not supported.)
      - On the RZ/T2N Evaluation Board, ETH0, ETH1 and ETH2 can only be used in RGMII mode.

    The following IP addresses are assigned to each port on the board side.

      | Port | IP address     |
      |------|----------------|
      | ETH0 | 192.168.10.100 |
      | ETH1 | 192.168.20.100 |
      | ETH2 | 192.168.30.100 |


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0
      J-Link              : v9.44

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board      : RZ/T2N Evaluation board  x 1
    - MPU               : RZ/T2N
    - Target Core       : Cortex-A55 Core0
    - Operating frequency : 1200 MHz
    - Boot mode         : RAM execution

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A
    - 3 x Ethernet cable (one for each of ETH0, ETH1 and ETH2)
    - A host PC with 3 Ethernet ports, when evaluating 3-port simultaneous communication


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - Use Ethernet
        SW7.2   : ON
        SW10.2  : ON
        SW12.3  : ON
        SW12.4  : ON
        J15     : 1-2 short
        J41     : 2-3 short


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

    Connect the Ethernet cables between the Ethernet ports of the host PC and the "ETH0",
    "ETH1" and "ETH2" connectors on the board.

      | Board connector | Board IP address | Host PC side                         |
      |-----------------|------------------|--------------------------------------|
      | ETH0            | 192.168.10.100   | set to the 192.168.10.x network      |
      | ETH1            | 192.168.20.100   | set to the 192.168.20.x network      |
      | ETH2            | 192.168.30.100   | set to the 192.168.30.x network      |

    The IP address of the Ethernet port on the host PC side connected to each port must be set
    to the same network as the board side port.
    ETH3 and ETH4 are not used by this sample program.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - This sample program displays the test results on a PC (terminal software) via a COM port.
      Set the configuration of Terminal Emulator (Tera Term) as below.

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

    - Set the reception line feed code of the terminal software to [LF].
      Without this setting the log is not displayed correctly.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "ethernet_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[ethernet_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "ethernet_rzt2n_evk_ca55_0_ep\ethernet_rzt2n_evk_ca55_0_ep.eww" in the folder
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

  8. Select [Project]-[Download and Debug] from the EWARM menu.
     The program will break at the first code in "main".

  9. Select [Debug]-[Run] from the EWARM menu to execute the program.
     Initial settings of cores and registers in SSLB and the sample application program will be run.


  - e²studio environment

  1. Connect USB Micro-B to J32 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "ethernet_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[ethernet_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.


Execution of sample program:

  1. Confirm that the start-up log appears on the terminal software. It shows the MAC address
     and the IP address of each port, and that the link of port 0, 1 and 2 is up.

  2. Enter the "ping 192.168.10.100", "ping 192.168.20.100" and "ping 192.168.30.100" commands
     from the command prompt of the host PC.

  3. Confirm that the ping replies appear on the terminal software, with the port number, the
     transmit size, the IP address and the sequence number of each reply.