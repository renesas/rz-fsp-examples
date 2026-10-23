File Name   : readme.txt
Description : Readme of the RZ/T2N SPI example project (spi_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the basic functionality of the RZ/T2N SPI driver on the
    Renesas RZ/T2N Evaluation board.
    After the debug connection to the Evaluation board, the sample program initializes the SPI.
    Loopback communication is performed using SPI ch0 and ch2 of the same board. The two
    channels are wired to each other between the J34 and the J45 connectors, so only one
    Evaluation board is required.

    The main files created for the sample program are as below.
      - r_transfer_api.h           : The file containing transfer interface.
      - r_spi_api.h                : The file containing SPI interface.
      - r_spi.h                    : The file containing SPI instances.
      - r_spi.c                    : The file containing SPI driver.
      - r_spi_cfg.h                : The file containing configuration header for SPI.
      - hal_data.h, hal_data.c     : The file containing configuration parameter for SPI.
      - hal_entry.c                : The file containing sample application of SPI.

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as hal_entry().
      2. R_SPI_Open() initializes SPI settings.
      3. R_SPI_WriteRead() is executed, and start a sending and receiving transfer.
      4. Wait for SPI_EVENT_TRANSFER_COMPLETE callback event.
      5. R_SPI_Write() and R_SPI_Read() are executed, and start a master sending and slave
         receiving transfer.
      6. Wait for SPI_EVENT_TRANSFER_COMPLETE callback event.
      7. R_SPI_Read() and R_SPI_Write() are executed, and start a master receiving and slave
         sending transfer.
      8. Wait for SPI_EVENT_TRANSFER_COMPLETE callback event.


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
    - 4 x jumper wire (for connecting SPI ch0 on J34 and SPI ch2 on J45)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - Use SPI ch0
        SW7.3   : OFF
        SW7.5   : OFF
        E8      : Short
        E9      : Short
        E10     : Open
        E11     : Open

    - Use SPI ch2
        SW7.7   : OFF
        SW7.8   : OFF

    E8 to E11 are solder jumpers on the Evaluation board. Check their state before starting,
    and rework them if they do not match the settings above.


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

    Connect SPI ch0 and ch2 pins following. Both channels are on the same board, ch0 on the J34
    connector and ch2 on the J45 connector, so the four jumper wires are connected within one
    board.

      Pins   : SPI ch0 - SPI ch2
      ---------------------------
      SSL    : J34-1   - J45-6
      MOSI   : J34-2   - J45-7
      MISO   : J34-3   - J45-8
      RSPCK  : J34-4   - J45-3

    Connect the same signal name to the same signal name. Do not cross MOSI and MISO.
    Connect the jumper wires while the power of the board is OFF.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - In the sample program, MRCLK is selected in the configuration settings (MRCLK/MRIOCLK
      selection), but please check the selection of MRCLK and MRIOCLK depending on the board
      design.

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

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "spi_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[spi_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "spi_rzt2n_evk_cr52_0_ep\spi_rzt2n_evk_cr52_0_ep.eww" in the folder
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
    - Check the [select archive file] and select "spi_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[spi_rzt2n_evk_cr52_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.