File Name   : readme.txt
Description : Readme of the RZ/T2N IIC (Slave) example project
              (iic_slave_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project demonstrates the basic functionality of the RZ/T2N IIC (Slave) driver
    on the Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program initializes the
    IIC (Slave) function, and sends and receives data in response to requests from the
    IIC (Master). The data received from the master ("abcd") is sent back to the master.

    Two Evaluation boards are required. This project runs on the slave side board, and the
    IIC (Master) example project (iic_master_rzt2n_evk_ca55_0_ep) runs on the counterpart board.

    The main files created for the sample program are as below.
      - r_i2c_slave_api.h          : The file containing IIC(Slave) API interface.
      - r_iic_slave.h              : The file containing IIC(Slave) instances.
      - r_iic_slave.c              : The file containing IIC(Slave) driver.
      - r_iic_slave_cfg.h          : The file containing configuration header for IIC(Slave).
      - hal_data.h, hal_data.c     : The file containing configuration parameter for IIC(Slave).
      - hal_entry.c                : The file containing sample application of IIC(Slave).

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as hal_entry().
      2. R_IIC_SLAVE_Open() initializes IIC(Slave) settings.
      3. R_IIC_SLAVE_Read() is executed, receive data from IIC(Master).
      4. After, send data back in response to requests from IIC(Master).


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board x 2
                     ( board(1) : IIC Master (iic_master_rzt2n_evk_ca55_0_ep)
                       board(2) : IIC Slave (this project) )
    - Target Core  : Cortex-A55 Core0
    - Boot mode    : RAM execution

    * Prepare the following for EACH board (2 sets in total).
    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A

    * Prepare the following to connect the two boards.
    - 3 x jumper wire (for SCL, SDA and GND between the J47 connectors of the two boards)


4. Hardware Configurations:

    Configure BOTH Evaluation boards by switches and jumper blocks as below.
    The master side and the slave side use exactly the same settings.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - Use IIC
        SW13.3  : OFF
        SW13.4  : ON
        SW13.5  : OFF
        SW13.8  : OFF
        SW13.9  : ON
        SW13.10 : OFF

    The SW13 settings route the IIC signals to P15_4 (SCL) and P15_5 (SDA).


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

    Connect the IIC (Master) side board to J47 of this board with three jumper wires.

      Example of connection to RZ/T2N Evaluation board
       Pins : Master side(T2N) - Slave side(T2N)
      -------------------------------------------
       SCL  : J47-25(P15_4)    - J47-25(P15_4)
       SDA  : J47-27(P15_5)    - J47-27(P15_5)
       GND  : J47-29           - J47-29

        RZ/T2N board(1)                      RZ/T2N board(2)
        IIC Master                           IIC Slave

        J47-25 (SCL, P15_4) O------------- O J47-25 (SCL, P15_4)
        J47-27 (SDA, P15_5) O------------- O J47-27 (SDA, P15_5)
        J47-29 (GND)        O------------- O J47-29 (GND)

    The pins are connected straight through. Do not cross SCL and SDA.
    Do not forget the GND wire; without it the two boards have no common reference and the
    communication does not work.
    No external pull-up resistor has to be added; use the wiring above as it is.

    Connect the jumper wires while the power of both boards is OFF.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply of both boards.


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

    - Start this IIC (Slave) side first, before the IIC (Master) side.
      After it is resumed, the slave stays in R_IIC_SLAVE_Read() waiting for a request from the
      master. It looks as if the program has stopped, but this is the expected state. Leave it
      running and start the master.

    - The example project passes only when all of the following are complete. It is not enough
      that R_IIC_SLAVE_Open() returns FSP_SUCCESS.
        1. Master write complete
        2. Slave receive complete
        3. Slave response complete
        4. Master read complete
        5. The data sent by the master ("abcd") and the data received back are the same

    - If the master waits forever, check the following in order.
        1. Was this slave side started before the master ?
        2. Is J47-25 of the master connected to J47-25 of the slave ?
        3. Is J47-27 of the master connected to J47-27 of the slave ?
        4. Is J47-29 (GND) connected between the two boards ?
        5. Are the SW13 settings the same on both boards ?
        6. Did R_IIC_SLAVE_Open() succeed on the slave side ?
        7. Did R_IIC_MASTER_Open() succeed on the master side ?
        8. Does the slave address set in this project match the target address of the master
           project ? Check it in the configuration of both projects.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "iic_slave_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  Note (two boards are used at the same time)
  - Two debug sessions are required, one for the master project and one for the slave project.
    The two projects may be opened in different IDEs (for example the master in e²studio and
    the slave in IAR EWARM); the code generation and the build of the two projects can be done
    in parallel.
  - Start the debug session of this IIC (Slave) side first, and then the IIC (Master) side.
  - When the two boards are connected to the same PC, two emulators are recognized. Select the
    emulator of the target board before starting the debug session.

  The recommended order of the whole procedure is as below.

    1. Turn off the power of both boards.
    2. Connect J47-25, J47-27 and J47-29 of the two boards with three jumper wires.
    3. Check the SW13 settings of both boards.
    4. Turn on the power of both boards.
    5. Start debugging this slave project and resume it. It waits for the master.
    6. Start debugging the master project and resume it.
    7. The master sends "abcd".
    8. This slave receives "abcd" and sends it back.
    9. The master receives "abcd" and the verification passes.


  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[iic_slave_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "iic_slave_rzt2n_evk_ca55_0_ep\iic_slave_rzt2n_evk_ca55_0_ep.eww" in the folder
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
    - Check the [select archive file] and select "iic_slave_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[iic_slave_rzt2n_evk_ca55_0_ep Debug_Flat
     (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.
