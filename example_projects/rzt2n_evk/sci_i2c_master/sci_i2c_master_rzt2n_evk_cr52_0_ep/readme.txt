File Name   : readme.txt
Description : Readme of the RZ/T2N SCI_IIC (Master) example project
              (sci_i2c_master_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the basic functionality of the RZ/T2N SCI_IIC (Master)
    driver on the Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program initializes the
    SCI_IIC (Master) function, sends "abcd" from the SCI_IIC (Master) to the IIC (Slave), and
    after that receives "abcd" back from the IIC (Slave).

    Two Evaluation boards are required. This project runs on the master side board, and the
    IIC (Slave) example project (iic_slave_rzt2n_evk_cr52_0_ep) runs on the counterpart board.

    The main files created for the sample program are as below.
      - r_i2c_master_api.h         : The file containing IIC(Master) API interface.
      - r_sci_i2c.h                : The file containing SCI_IIC(Master) instances.
      - r_sci_i2c.c                : The file containing SCI_IIC(Master) driver.
      - r_sci_i2c_cfg.h            : The file containing configuration header for SCI_IIC(Master).
      - hal_data.h, hal_data.c     : The file containing configuration parameter for
                                     SCI_IIC(Master).
      - hal_entry.c                : The file containing sample application of SCI_IIC(Master).

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as hal_entry().
      2. R_SCI_I2C_Open() initializes SCI_IIC(Master) settings.
      3. R_SCI_I2C_Write() is executed, send data to IIC(Slave).
      4. R_SCI_I2C_Read() is executed, receive data back from the IIC(Slave).
      5. Verify send and receive data.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GNU ARM Embedded (Version:13.3.1.arm-13-24)
      RZ FSP version   : 4.2.0


    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board x 2
                     ( board(1) : SCI_IIC Master (this project)
                       board(2) : IIC Slave (iic_slave_rzt2n_evk_cr52_0_ep) )
    - Target Core  : Cortex-R52 CPU0
    - Boot mode    : RAM execution

    * Prepare the following for EACH board (2 sets in total).
    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A

    * Prepare the following to connect the two boards.
    - 3 x jumper wire (for SCL, SDA and GND between J45 of the master board and J47 of the
      slave board)


4. Hardware Configurations:

    The master side and the slave side do NOT use the same settings. Configure each board as
    below.

    * Both boards
    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    * board(1), the SCI_IIC Master side (this project)
    - Use SCI_IIC(Master)
        SW7.7   : OFF
        SW7.8   : OFF
        J39     : 2-3 short

    * board(2), the IIC Slave side
    - Use IIC
        SW13.3  : OFF
        SW13.4  : ON
        SW13.5  : OFF
        SW13.8  : OFF
        SW13.9  : ON
        SW13.10 : OFF

      Refer to the readme.txt of the IIC (Slave) example project for the slave side settings.


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

    Connect the IIC (Slave) side board with three jumper wires. Note that the master side uses
    the J45 connector while the slave side uses the J47 connector, so the pin numbers are
    different on the two sides.

      Example of connection to RZ/T2N Evaluation board
       Pins : Master side(T2N) - Slave side(T2N)
      -------------------------------------------
       SCL  : J45-8(P05_3)     - J47-25(P15_4)
       SDA  : J45-7(P05_4)     - J47-27(P15_5)
       GND  : J45-1            - J47-29

        RZ/T2N board(1)                      RZ/T2N board(2)
        SCI_IIC Master                       IIC Slave

        J45-8 (SCL, P05_3)  O------------- O J47-25 (SCL, P15_4)
        J45-7 (SDA, P05_4)  O------------- O J47-27 (SDA, P15_5)
        J45-1 (GND)         O------------- O J47-29 (GND)

    Connect SCL to SCL and SDA to SDA. Do not cross them.
    On the master side the SCL pin number (J45-8) is larger than the SDA pin number (J45-7),
    while on the slave side the SCL pin number (J47-25) is smaller than the SDA pin number
    (J47-27). Check the signal name of each pin, not the pin order.
    Do not forget the GND wire; without it the two boards have no common reference and the
    communication does not work.
    No external pull-up resistor has to be added; use the wiring above as it is.

    Note for users who have already run the IIC (Master) example project.
    The slave side board is exactly the same and keeps running the IIC (Slave) example project
    with the same SW13 settings and the same J47 wiring. The master side does not use the IIC
    peripheral and the J47 connector any more; it uses the SCI in I2C mode and brings SCL and
    SDA out to the J45 connector. Only the three leads on the master side have to be moved.

      | Signal | IIC Master (before) | SCI_IIC Master (after) | Slave side  |
      |--------|---------------------|------------------------|-------------|
      | SCL    | J47-25 (P15_4)      | J45-8 (P05_3)          | J47-25, unchanged |
      | SDA    | J47-27 (P15_5)      | J45-7 (P05_4)          | J47-27, unchanged |
      | GND    | J47-29              | J45-1                  | J47-29, unchanged |

    In addition to moving the leads, set SW7.7 and SW7.8 of the master board to OFF and set J39
    of the master board to 2-3 short. The boot settings (SW8, SW10.7, and SW10.10) do not change.

    The API of the master side changes as below, but the functional test is the same: the master
    sends "abcd" and has to receive "abcd" back from the slave.

      | IIC (Master)          | SCI_IIC (Master)  |
      |-----------------------|-------------------|
      | R_IIC_MASTER_Open()   | R_SCI_I2C_Open()  |
      | R_IIC_MASTER_Write()  | R_SCI_I2C_Write() |
      | R_IIC_MASTER_Read()   | R_SCI_I2C_Read()  |

    Connect the jumper wires while the power of both boards is OFF.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply of both boards.


Note:

    - This sample program displays the startup log on a PC (terminal software) via a
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

    - Start the IIC (Slave) side first.
      The slave stays in R_IIC_SLAVE_Read() waiting for a request from the master. It looks as
      if the program has stopped, but this is the expected state. After the slave is running,
      start the master.

    - A convenient place for a breakpoint on the master side is the "while(1);" statement at the
      end of hal_entry(). If the program reaches that breakpoint, the whole sequence (write,
      slave response, read and verification) has completed.

    - The example project passes only when all of the following are complete. It is not enough
      that R_SCI_I2C_Open() returns FSP_SUCCESS.
        1. Master write complete
        2. Slave receive complete
        3. Slave response complete
        4. Master read complete
        5. The data sent by the master ("abcd") and the data received back are the same

    - If the master waits forever, check the following in order.
        1. Was the slave side started before the master ?
        2. Is J45-8 of the master connected to J47-25 of the slave ?
        3. Is J45-7 of the master connected to J47-27 of the slave ?
        4. Is the GND connected between J45-1 of the master and J47-29 of the slave ?
        5. Are SW7.7 and SW7.8 OFF and J39 2-3 short on the master board, and are the SW13
           settings correct on the slave board ?
        6. Did R_IIC_SLAVE_Open() succeed on the slave side ?
        7. Did R_SCI_I2C_Open() succeed on the master side ?
        8. Does the slave address set in this project match the address of the slave project ?
           Check it in the configuration of both projects.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "sci_i2c_master_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  Note (two boards are used at the same time)
  - Two debug sessions are required, one for this master project and one for the slave project.
    The two projects may be opened in different IDEs (for example the master in e²studio and
    the slave in IAR EWARM); the code generation and the build of the two projects can be done
    in parallel.
  - Start the debug session of the IIC (Slave) side first, and then this SCI_IIC (Master) side.
  - When the two boards are connected to the same PC, two emulators are recognized. Select the
    emulator of the target board before starting the debug session.

  The recommended order of the whole procedure is as below.

    1. Turn off the power of both boards.
    2. Connect J45-8, J45-7 and J45-1 of the master board to J47-25, J47-27 and J47-29 of the
       slave board with three jumper wires.
    3. Check SW7.7, SW7.8 and J39 on the master board and SW13 on the slave board.
    4. Turn on the power of both boards.
    5. Start debugging the slave project and resume it. It waits for the master.
    6. Start debugging this master project and resume it.
    7. The master sends "abcd".
    8. The slave receives "abcd" and sends it back.
    9. The master receives "abcd" and the verification passes.


  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[sci_i2c_master_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "sci_i2c_master_rzt2n_evk_cr52_0_ep\sci_i2c_master_rzt2n_evk_cr52_0_ep.eww"
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
    - Check the [select archive file] and select "sci_i2c_master_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[sci_i2c_master_rzt2n_evk_cr52_0_ep Debug_Flat
     (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.