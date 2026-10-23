File Name   : readme.txt
Description : Readme of the RZ/T2N SCI_SPI example project (sci_spi_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project demonstrates the basic functionality of the RZ/T2N SCI_SPI driver on
    the Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program initializes the
    SCI_SPI. Loopback communication is performed using SCI_SPI ch1 and ch2 of the same board.
    SCI_SPI ch1 works as the slave and SCI_SPI ch2 works as the master, and the two channels are
    wired to each other on the J45 connector. Only one Evaluation board is required.

    The main files created for the sample program are as below.
      - r_transfer_api.h           : The file containing transfer interface.
      - r_spi_api.h                : The file containing SPI interface.
      - r_sci_spi.h                : The file containing SCI_SPI instances.
      - r_sci_spi.c                : The file containing SCI_SPI driver.
      - r_sci_spi_cfg.h            : The file containing configuration header for SCI_SPI.
      - hal_data.h, hal_data.c     : The file containing configuration parameter for SCI_SPI.
      - hal_entry.c                : The file containing sample application of SCI_SPI.

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as hal_entry().
      2. Configure Slave Select Line.
      3. R_SCI_SPI_Open() initializes SCI_SPI settings.
      4. R_SCI_SPI_WriteRead() is executed, starting the slave transmit and receive transfer.
      5. The flow of transmission and reception on the master side is as follows.
         a. Assert Slave Select Line.
         b. R_SCI_SPI_WriteRead() is executed, starting the master transmit and receive transfer.
         c. Wait for SCI_SPI_MASTER_EVENT_TRANSFER_COMPLETE callback event.
         d. De-assert Slave Select Line.
      6. Wait for SCI_SPI_SLAVE_EVENT_TRANSFER_COMPLETE callback event.
      7. Following steps 3 to 5, R_SCI_SPI_Read() and R_SCI_SPI_Write() are executed, starting
         the master transmit and slave receive transfers.
      8. Following steps 3 to 5, R_SCI_SPI_Write() and R_SCI_SPI_Read() are executed, starting
         the master receiving and slave sending transfer.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board  x 1
    - Target Core  : Cortex-A55 Core0
    - Boot mode    : RAM execution

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A
    - 4 x jumper wire (for connecting SCI_SPI ch1 and ch2 on the J45 connector)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - Use SCI_SPI
        SW7.7   : OFF
        SW7.8   : OFF


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

    Connect SCI_SPI ch1 and ch2 pins following. Both channels are on the J45 connector of the
    same board, so the four jumper wires are connected within one board.

      Pins   : ch1(slave) - ch2(master)
      ---------------------------------
      SSL    : J45-11     - J45-5
      MOSI   : J45-12     - J45-7
      MISO   : J45-4      - J45-8
      RSPCK  : J45-2      - J45-3

    Connect the same signal name to the same signal name. Do not cross MOSI and MISO.
    Connect the jumper wires while the power of the board is OFF.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - This sample program displays the SCI_SPI test results on a PC (terminal software) via a
      COM port. Set the configuration of Terminal Emulator (Tera Term) as below.

        Port         : COM port provided by the RZ board.
                       (When connected to a PC via a USB serial port, you will see two COM ports.
                        Connect to the higher numbered COM port of the two ports.)
        Speed        : 115200 bps
        Data         : 8 bit
        Parity       : none
        Stop bits    : 1 bit
        Flow control : none

    - The higher numbered COM port is used on both the Cortex-R52 and the Cortex-A55 version of
      this example project, unlike the other example projects where the Cortex-A55 version uses
      the smaller numbered COM port. This is because SCI1 is assigned to the SCI_SPI ch1 (slave)
      of this example project and cannot be used for the console, so the console of both
      versions is output through SCI0, which is the higher numbered COM port.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "sci_spi_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[sci_spi_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "sci_spi_rzt2n_evk_ca55_0_ep\sci_spi_rzt2n_evk_ca55_0_ep.eww" in the folder
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
    - Check the [select archive file] and select "sci_spi_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[sci_spi_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.