File Name   : readme.txt
Description : Readme of the RZ/T2N xSPI_OSPI example project
              (xspi_ospi_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the xSPI (OSPI) of the RZ/T2N accessing the OctaFlash on
    the Renesas RZ/T2N Evaluation board.

    After a debug connection to the board, this sample program initializes xSPI and changes the
    OctaFlash on the board to OPI-DTR mode. Data is erased, written and read using the xSPI
    8D-8D-8D protocol.
    In the first half, Manual Command mode is used to read and write data. The second half uses
    the Write API function and Memory-mapping mode to read and write data.

    The main files created for the sample program are as below.
      - r_spi_flash_api.h          : The file containing SPI_Flash interface.
      - r_xspi_ospi.h              : The file containing xSPI_OSPI instances.
      - r_xspi_ospi.c              : The file containing xSPI_OSPI driver.
      - r_xspi_ospi_cfg.h          : The file containing configuration header for xSPI_OSPI.
      - hal_data.h, hal_data.c     : The file containing configuration parameter for xSPI_OSPI.
      - hal_entry.c                : The file containing sample application of xSPI_OSPI.

    The details of the operation flow are shown below.

       1. Start the debugging and sample program as hal_entry().

       2. R_XSPI_OSPI_Open() initializes xSPI as a 1S-1S-1S protocol operation.

       3. ospi_set_DTR_OPI_Mode_enable() sets OctaFlash's Dummy cycle to 10 and changes the
          operating mode to OPI-DTR mode. At the same time, change xSPI to 8D-8D-8D protocol
          operation.

       4. R_XSPI_OSPI_Erase() performs sector erase processing and deletes the data
          written to OctaFlash.

       5. After issuing the WriteEnable command ("06F9h"), R_XSPI_OSPI_DirectTransfer() issues
          the Page Program (PP) command ("12EDh") and writes the data "0x1114" to the OctaFlash
          address "0x00001114".

       6. R_XSPI_OSPI_DirectTransfer() issues the OCTA DTR Read Mode (8DTRD) command ("EE11h")
          and reads the data from the OctaFlash address "0x00001114".

       7. Compare the data written in (step 5) with the data read in (step 6).

       8. Compare the data written in (step 5) with the data read in Memory-mapping mode.

       9. R_XSPI_OSPI_Write() writes 8 bytes of data from the OctaFlash address "0x00001000".

      10. Compare the data written in (step 9) with the data read in Memory-mapping mode.


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

    The OctaFlash used by this sample program is mounted on the Evaluation board. No external
    memory device is required.


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - Use XSPI0
        SW7.6   : ON

    Note that the boot device is the xSPI1 serial flash while this sample program accesses the
    OctaFlash on xSPI0. They are different devices, so the erase and write of this sample
    program does not destroy the boot data.


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

    The OctaFlash accessed by this sample program is on the board and is connected to xSPI0.
    No external connection is required to check the operation.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


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

    - This sample program erases the OctaFlash on the board. The data written on the OctaFlash
      before running the program is lost.

    - This sample uses sector erase at step 4. Chip erase is not implemented in the supplied
      source.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "xspi_ospi_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[xspi_ospi_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "xspi_ospi_rzt2n_evk_cr52_0_ep\xspi_ospi_rzt2n_evk_cr52_0_ep.eww" in the folder
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
    - Check the [select archive file] and select "xspi_ospi_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[xspi_ospi_rzt2n_evk_cr52_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.