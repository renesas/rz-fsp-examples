File Name   : readme.txt
Description : Readme of the RZ/T2N FreeRTOS-Plus-FAT (SDHI) example project
              (FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the FreeRTOS-Plus-FAT file system on a microSD card
    accessed through the SDHI of the RZ/T2N, on the Renesas RZ/T2N Evaluation board.

    After starting debugging, if you connect a memory device, the host will write a TESTFILE.txt
    file to the memory device. After writing, the TESTFILE.txt file is read back and verified.
    The content of TESTFILE.txt is a list of 10 kB character data (ASCII code 33 to 126,
    10240 bytes).

    The details of the operation flow are shown below.

      1. Start the debugging and sample program as hal_entry().

      2. Create file data (ASCII code 33 to 126, 10240 bytes) and init read buffer.

      3. R_SCI_UART_Open() initializes SCI0 as UART settings.

      4. RM_FREERTOS_PLUS_FAT_Open() is executed and initializes lower layer media device
         settings.

      5. RM_FREERTOS_PLUS_FAT_MediaInit() is executed and initialize the media and the disk.

      6. RM_FREERTOS_PLUS_FAT_DiskInit() is executed and initialize one disk for each partition
         used in the application.

      7. The terminal will display the following and wait for your input.

           [Terminal Display]
            [INFO] To mount the microSD card, it needs to be partitioned and formatted.
            [PROMPT] Do you want to format the microSD card ?
              [1] Format the microSD card (FAT32)
              [2] Mount without formatting
              [3] Cancel and exit program
            Enter your choice >

      8. Program processing changes depending on the value you enter.
         When [1] is selected, the case is STATE_FORMAT.
         When [2] is selected, the case is STATE_MOUNT.
         When [3] is selected, the case is STATE_REMOVE.

           Case : STATE_FORMAT
                  1. FF_Partition() is executed and the partitions of a disk are
                     created/initialized.
                  2. FF_FormatDisk() is executed and a partition of a disk is formatted.
                  3. Set the g_state variable to "STATE_MOUNT".

           Case : STATE_MOUNT
                  1. FF_Mount() is executed and mounts by the Specified partition,
                     (FF_IOManager_t object) provided.
                  2. If the mount fails, set the g_state variable to "STATE_REMOVE".
                     If the mount succeeds, set the g_state variable to "STATE_DATA_WRITE".

           Case : STATE_DATA_WRITE
                  1. FF_FS_Add() is executed and adds a mounted partition to the
                     FreeRTOS-Plus-FAT virtual file system.
                  2. ff_fopen() is executed and open a source file(TESTFILE.txt) for writing.
                  3. If opening the file fails, set the g_state variable to "STATE_FINISH".
                     If opening the file succeeds, ff_fwrite() is executed and the file data is
                     written.
                  4. ff_fclose() is executed and the file is closed.
                  5. Set the g_state variable to "STATE_FILE_READ".

           Case : STATE_FILE_READ
                  1. ff_fopen() is executed and open a source file(TESTFILE.txt) for reading.
                  2. If opening the file fails, set the g_state variable to "STATE_FINISH".
                     If opening the file succeeds, ff_fread() is executed and the file data is
                     read.
                  3. ff_fclose() is executed and the file is closed.
                  4. Verify the read data.
                  5. If the verification is successful, set the g_state variable to
                     "STATE_REMOVE".

           Case : STATE_REMOVE
                  1. FF_Unmount () is executed, unmounts the active partition.
                  2. Set the g_state variable to "STATE_FINISH".

           Case : STATE_FINISH
                  1. The end of the program is displayed on the terminal.


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
    - 1 x microSD card (the sample program can format it as FAT32)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - Use microSD Slot (J27)
        SW7.1   : ON
        J25     : 3-4 short


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

    Insert the microSD card into the microSD slot (J27) of the Evaluation board.
    Do not insert or remove the microSD card while the sample program is running.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - This sample program displays the FreeRTOS-Plus-FAT (SDHI) test results on a PC (terminal
      software) via a COM port. Set the configuration of Terminal Emulator (Tera Term) as below.

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

    - If [1] is selected at the prompt, the microSD card is partitioned and formatted as FAT32.
      All the data on the microSD card is lost. Use a card that may be erased.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep\FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep.eww"
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
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[FreeRTOSplusFAT_rzt2n_evk_cr52_0_ep Debug_Flat
     (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.