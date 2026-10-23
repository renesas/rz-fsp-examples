
File Name   : readme.txt
Description : Readme of the RZ/T2N PCIe Endpoint example project
              (pcie_ep_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project is the Endpoint side of a PCIe communication demonstration
    between two Renesas RZ/T2N Evaluation boards. The corresponding Root Complex project
    is "pcie_rc_rzt2n_evk_ca55_0_ep".

    Two boards are required:
      - Board(1) : Root Complex (RC Device)
        Project  : pcie_rc_rzt2n_evk_ca55_0_ep

      - Board(2) : Endpoint (EP Device)
        Project  : pcie_ep_rzt2n_evk_ca55_0_ep

    The two boards are connected through J6 (PCIe x1). The EP Device must be started first,
    and then the RC Device is started on a second IDE instance. The RC Device performs PCIe
    configuration access, memory read/write, DMAC transfer, and transfer speed measurement
    tests against the EP Device.

    The main files created for the sample program are as below.
      - r_pcie_rc_dmac.h:
        The file containing PCIe DMAC instances.

      - r_pcie_rc_dmac.c:
        The file containing PCIe DMAC driver.

      - r_pcie_rc_apl.c (or r_pcie_ep_apl.c), hal_entry.c:
        The file containing sample application of PCIe.

    Each functional test can be enabled or disabled by setting the corresponding macro
    variable to 1 or 0 in r_pcie_rc_apl.c and rebuilding the project.

      | func No. | Macro variable                   | Test details                                                             |
      |----------|----------------------------------|--------------------------------------------------------------------------|
      | func 1.  | PCI_CONFIG_READ_WRITE_TEST       | Read EP device configuration register from RC device                     |
      | func 2.  | PCI_CONFIG_READ_WRITE_TEST       | Write EP device configuration register from RC device                    |
      | func 3.  | PCI_MEMORY_READ_WRITE_TEST       | Read/write EP device memory from RC device                               |
      | func 4.  | PCI_DMAC_READ_WRITE_TEST         | Read and write EP device memory from RC device using DMAC                |
      | func 5.  | PCI_MEMORY_READ_WRITE_SPEED_TEST | Measure memory read and write speed from RC device to EP device          |
      | func 6.  | PCI_DMAC_READ_WRITE_SPEED_TEST   | Measure memory read and write speed from RC device to EP device using DMAC |

    Macro variables for pcie_rc_rzt2n_evk_ca55_0_ep:

      | Macro variable                   | Default     | Macro variable details                                                          |
      |----------------------------------|-------------|---------------------------------------------------------------------------------|
      | PCIE_DMAC_CH                     | 0x0         | DMAC channel number for PCIe module (0 - 7).                                    |
      | PCI_CONFIG_READ_WRITE_TEST       | 0x1         | Enables or disables the R/W test of configuration settings.                     |
      | PCI_MEMORY_READ_WRITE_TEST       | 0x1         | Enables or disables the CA55 memory R/W test (DMAC not used).             |
      | PCI_DMAC_READ_WRITE_TEST         | 0x1         | Enables or disables the memory R/W test using DMAC.                             |
      | PCI_MEMORY_READ_WRITE_SPEED_TEST | 0x1         | Enables or disables the speed measurement test for memory R/W (DMAC not used).  |
      | PCI_DMAC_READ_WRITE_SPEED_TEST   | 0x1         | Enables or disables the speed measurement test for memory R/W using DMAC.       |
      | PCI_DMAC_WRITE_ADDR              | 0x10110000  | DMAC R/W test data storage destination address (DMAC source address).           |
      | PCIE_TARGET_WINDOW_MIRROR_ADDR   | 0xA0000000  | CPU's mirror base address for accessing the PCIe window configured by the RC.   |
      | PCIE_TARGET_WINDOW_SIZE          | 0x1000      | Size of the PCIe window.                                                        |
      | PCIE_NOTIFY_FLAG_OFFSET          | 0x00        | Offset for the notification flag.                                               |
      | PCIE_TEST_DATA_OFFSET            | 0x04        | Address offset for the test data area.                                          |
      | PCIE_TEST_DATA_ADDR              | 0xA0000004  | Address in RC's PCIe mirror window where write test data to the EP's AXI window.|
      | PCIE_TEST_DATA_SIZE              | 0xFFC       | Size of the test data area.                                                     |
      | RC_TO_EP_NOTIFY_ADDR             | 0xA0000000  | Address where the notification flag from RC to EP is written.                   |
      | RC_TO_EP_NOTIFY_VALUE            | 0x52434550  | RC to EP notification flag ("RCEP").                                            |
      | EP_TO_RC_NOTIFY_ADDR             | 0x10190000  | Address where the notification flag from EP to RC is written.                   |
      | EP_TO_RC_NOTIFY_VALUE            | 0x45505243  | EP to RC notification flag ("EPRC").                                            |
      | EP_TO_RC_WRITEBACK_ADDR          | 0x10190004  | Address in RC's AXI window where data written back from EP is stored.           |

    Macro variables for pcie_ep_rzt2n_evk_ca55_0_ep:

      | Macro variable                   | Default     | Macro variable details                                            |
      |----------------------------------|-------------|-------------------------------------------------------------------|
      | PCIE_TARGET_WINDOW_MIRROR_ADDR   | 0xA0000000  | Mirror address of the PCIe area.                                  |
      | PCIE_TARGET_WINDOW_SIZE          | 0x1000      | Size of the PCIe window.                                          |
      | PCIE_EP_AXI_WINDOW_OFFSET        | 0x10000     | Offset of the EP window.                                          |
      | PCIE_TEST_DATA_OFFSET            | 0x04        | Address offset for the test data area.                            |
      | RC_TO_EP_NOTIFY_ADDR             | 0x10180000  | Address where the notification flag from RC to EP is written.     |
      | RC_TO_EP_NOTIFY_VALUE            | 0x52434550  | RC to EP notification flag ("RCEP").                              |
      | EP_TO_RC_NOTIFY_ADDR             | 0xA0010000  | Address where the notification flag from EP to RC is written.     |
      | EP_TO_RC_NOTIFY_VALUE            | 0x45505243  | EP to RC notification flag ("EPRC").                              |
      | EP_WRITEBACK_SIZE                | 0xFFC       | Size of the test data area.                                       |
      | EP_WRITEBACK_DST_ADDR            | 0xA0010004  | Destination address where writeback data from EP to RC is stored. |
      | EP_WRITEBACK_SRC_ADDR            | 0x10180004  | Source address from which writeback data from EP to RC is read.   |

    The details of the operation flow are shown below.

      0. Before starting the debug sessions, check the following.
         - The two boards are connected with the PCIe cable
           (J6 of board(1) - J6 of board(2)).
         - The switches of each board are set for the RC Device / EP Device respectively.
         - Both boards are powered on.
         - Terminal software is connected to each COM port.

      1. Window : pcie_ep_rzt2n_evk_ca55_0_ep
         Start debugging the EP Device.

      2. R_SCI_UART_Open() is executed and initializes the SCI1 settings.

      3. Initialize the PCIE_EP related modules.

      4. The following menu is displayed on the EP Device terminal.

            [EP device terminal]
               * Display AXI Window of EP Device
               [1] = Scan start.
               [2] = The end of the program.
               Select No.>

         If "1" is entered, the data in the AXI Window is output.
         If "2" is entered, the program terminates.
         For any other input, "Invalid input. Please enter 1 or 2." is displayed and the
         menu is shown again.

         The EP Device continuously monitors address 0x10180000 for the RC notification
         value 0x52434550. When the notification is detected, the EP Device copies data
         from 0x10180004 to 0xA0010004 and then writes 0x45505243 to 0xA0010000 to notify
         the RC Device that the writeback operation has completed.

      5. Change Window : pcie_rc_rzt2n_evk_ca55_0_ep
         Start debugging the RC Device on the second IDE instance while the EP Device
         remains running.

      6. R_SCI_UART_Open() is executed and initializes the SCI1 settings.

      7. Initialize the PCIE_RC related modules.

      8. R_PCIE_RC_LinkStatusGet() is executed and the RC Device waits for link-up with
         the EP Device.

      9. After completing each device setting, link up the RC Device and EP Device.

     10. func1 starts.
         Read the following EP Device configuration registers and display them on the
         RC Device terminal.
           - PCI_EP_RID_CC_F0
           - PCI_EP_SUBSID_F0

     11. func2 starts.
         Set bit10 of the PCI_EP_COM_STA_F0 register of the EP Device to "1".
         The register values before and after the write are displayed on the RC terminal.

     12. func3 starts.
         The RC Device writes data directly to the memory region beginning at 0xA0000004
         (up to 0xFFC bytes), which maps to the EP Device AXI Window.
         After the write, the RC Device writes 0x52434550 to 0xA0000000.
         The RC Device then waits for the EP writeback completion value 0x45505243 at
         0x10190000. After receiving the notification, the RC Device clears the initial
         notification flag at 0xA0000000 and verifies the data written back by the EP
         Device to 0x10190004.
         "Transfer Completed!!" is displayed when the transfer completes successfully.

     13. func4 starts.
         Test data is prepared in the RC Device local buffer at 0x10110000.
         A DMAC transfer copies 0x1000 bytes from the RC local buffer to the EP Device
         AXI Window. Another DMAC transfer reads the data back to the RC Device local
         buffer beginning at 0x10110000 + 0x1000. The original and read-back data are
         then compared.
         "Transfer Completed!!" is displayed when the transfer completes successfully.

     14. func5 starts.
         Perform the same operation as func3 and measure the memory read/write speed.
         The RC Device then notifies the EP Device of completion, waits for the EP
         writeback notification, clears its notification flag, and verifies the
         writeback data.

     15. func6 starts.
         Perform the same operation as func4 and measure the DMAC read/write speed.

     16. PCIe func1 to func6 confirmation is completed.

    Step 12 - 15 Note:

      If memory transfer fails, check the macro variables or FSP settings in both projects
      and make sure they comply with the following rules.

        1. Make "RC_WINDOWS_MASK" of the RC project and "EP_WINDOWS_MASK" of the EP project equal.

        2. Make "RC_PW_DEST_ADDR" of the RC project and "EP_AW_BASE_ADDR" of the EP project equal.

        3. Do not set each Window to a reserved area.

      PCIe Window / AXI Window examples:

        Example 1)
          RC project:
            RC_PW_BASE_ADDR  : 0x400000000
            RC_PW_DEST_ADDR  : 0x10100000
            RC_WINDOWS_MASK  : 0x00000FFF

          EP project:
            EP_AW_BASE_ADDR   : 0x10100000
            EP_AW_DEST_ADDR   : 0x10100000
            EP_WINDOWS_MASK   : 0x00000FFF

          Result:
            Writing to RC Device memory 0xA0000000 - 0xA0000FFF writes to
            EP Device memory 0x10100000 - 0x10100FFF.

        Example 2)
          RC project:
            RC_PW_BASE_ADDR  : 0x400000000
            RC_PW_DEST_ADDR  : 0x10000000
            RC_WINDOWS_MASK  : 0x00000FFF

          EP project:
            EP_AW_BASE_ADDR   : 0x10000000
            EP_AW_DEST_ADDR   : 0x10001000
            EP_WINDOWS_MASK   : 0x00000FFF

          Result:
            Writing to RC Device memory 0xA0000000 - 0xA0000FFF writes to
            EP Device memory 0x10001000 - 0x10001FFF.

        Example 3)
          RC project:
            RC_PW_BASE_ADDR  : 0x400000000
            RC_PW_DEST_ADDR  : 0x10100000
            RC_WINDOWS_MASK  : 0x00000FFF

          EP project:
            EP_AW_BASE_ADDR   : 0x10100000
            EP_AW_DEST_ADDR   : 0x10101000
            EP_WINDOWS_MASK   : 0x00000FFF

          Result:
            Writing to RC Device memory 0xA0000000 - 0xA0000FFF writes to
            EP Device memory 0x10101000 - 0x10101FFF.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0

    - Terminal Emulator (Tera Term)


3. Hardware Requirements:

    - Target board : RZ/T2N Evaluation board  x 2
                     Board(1) : Root Complex
                     Board(2) : Endpoint

    - Target Core  : Cortex-A55 Core0
    - Boot mode    : RAM execution

    Prepare the following for EACH board (2 sets in total).
      - 1 x USB Micro-B cable (for J-Link OB)
      - 1 x USB Micro-B cable (for serial communication)
      - 1 x IAR I-jet (for debugging in IAR EWARM)
      - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC
        (for power supply)
      - 1 x AC adapter for USB supporting 15V/3A

    Prepare the following to connect the two boards.
      - 1 x PCI Express 1X to 1X riser extender cable, male to male
        Both ends are card edge connectors and are inserted into J6 of each board.


4. Hardware Configurations:

    Configure both RZ/T2N Evaluation boards as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - RZ/T2N Evaluation board(1) : pcie_rc_rzt2n_evk_ca55_0_ep
      Root Complex
        SW7.2   : OFF
        SW12.6  : ON
        SW12.7  : ON
        SW12.8  : ON

    - RZ/T2N Evaluation board(2) : pcie_ep_rzt2n_evk_ca55_0_ep
      Endpoint
        SW12.6  : OFF
        SW12.7  : OFF
        SW12.8  : OFF


5. Hardware Connections:

    This sample program requires two RZ/T2N Evaluation boards connected to each other
    through the PCIe connector (J6).

      RZ/T2N Evaluation board(1)                    RZ/T2N Evaluation board(2)
      [pcie_rc_rzt2n_evk_ca55_0_ep]                 [pcie_ep_rzt2n_evk_ca55_0_ep]
      Root Complex (RC Device)                      Endpoint (EP Device)

        J6 (PCIe x1) O=========== PCIe cable ===========O J6 (PCIe x1)

        J31 or J32 -- USB ------ PC (debugger)       J31 or J32 -- USB ------ PC (debugger)
        J33        -- USB ------ PC (COM port)       J33        -- USB ------ PC (COM port)
        J7         -- USB Type-C - AC adapter        J7         -- USB Type-C - AC adapter

    - Debugging Function Selection
      J29 : Open  (for debugging in e²studio, J-Link OB)
      J29 : Short (for debugging in IAR EWARM)

    Connection procedure:

      1. Make sure SW1 "POWER_SW" of both boards is OFF.
         Do not plug or unplug the PCIe cable while the boards are powered on.

      2. Insert one end of the PCIe cable into J6 of board(1), and the other end into
         J6 of board(2).

      3. Set the switches of each board as described in "4. Hardware Configurations".
         Board(1) must be configured as Root Complex and board(2) as Endpoint.

      4. Connect the USB cables and AC adapter of each board, and then turn on
         SW1 "POWER_SW" of both boards.


Note:

    - Two debug sessions are required, one for the EP project and one for the RC project.
      Start the IDE twice using a different workspace for each instance.
      Debug the EP project on the first instance and the RC project on the second instance.

    - Start the debug session of the EP Device first.
      After the menu is displayed on the EP Device terminal, start the RC Device debug
      session on the other IDE instance.
      If the RC Device is started first, it remains in R_PCIE_RC_LinkStatusGet() waiting
      for link-up and the test does not proceed.

    - When the two boards are connected to the same PC, two emulators and two pairs of
      COM ports are recognized. Select the emulator of the target board before starting
      the debug session and confirm that the terminal COM port belongs to the same board.

    - Configure the serial ports of both RC and EP Devices as below.

        Port         : COM port provided by each RZ board.
                       (When connected to a PC via a USB serial port, you will see two
                        COM ports for the corresponding board.
                        Connect to the smaller numbered COM port of the two ports.
                        If no characters are output to the terminal when the program is
                        running, try connecting the other COM.)
        Speed        : 115200 bps
        Data         : 8 bit
        Parity       : none
        Stop bits    : 1 bit
        Flow control : none

    - If one of the two boards is stopped or reset after link-up, restart the debug
      sessions of both boards from the beginning: EP Device first, then RC Device.
      The PCIe link is not established again automatically.

    - Before starting debug, please push the reset button on both boards to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset"
      button during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when
      warning markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking the project in
      Project Explorer, it can be deleted by [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Prepare two EWARM instances using different workspaces.
     Connect IAR I-jet to J31, USB Micro-B to J33, and USB Type-C cable with AC adapter
     to J7 for both boards.
     Connect the two boards through J6 using the PCIe cable.
     Switch SW1 "POWER_SW" from OFF to ON on both boards.

  2. Unzip the following archives to any folder.

       [iar]-[pcie_ep_rzt2n_evk_ca55_0_ep_Rev100.zip]
       [iar]-[pcie_rc_rzt2n_evk_ca55_0_ep_Rev100.zip]

  3. Start two IAR EWARM instances.

  4. Open the EP workspace on the first instance.

       "pcie_ep_rzt2n_evk_ca55_0_ep\pcie_ep_rzt2n_evk_ca55_0_ep.eww"

     Open the RC workspace on the second instance.

       "pcie_rc_rzt2n_evk_ca55_0_ep\pcie_rc_rzt2n_evk_ca55_0_ep.eww"

  5. Start the Smart Configurator from [Tools] for each project.
     The Smart Configurator must be registered in advance under [Tools]-[Configure Tools].

      Menu Text          : "FSP Smart Configurator"
      Command            : "$RASC_EXE_PATH$"
      Argument           : "--compiler IAR configuration.xml"
      Initial Directory  : "$PROJ_DIR$"

     Specify the absolute installation path of [rasc.exe] in "RASC_EXE_PATH"
     in [buildinfo.ipcf].

  6. Click [Generate Project Content] to generate the code in the Smart Configurator.

  7. Select [Project]-[Rebuild All] from the EWARM menu.

     - During the build, if a window appears searching for Smart Configurator, enter the
       number that indicates FSP v4.2.0 and press Enter.

     - If the build error "FSP version 4.x.x is not installed..." is displayed, make the
       project name and folder name the same and generate the code again.

       Before changing the file path:
         C:\..\pcie_xx_rzt2n_evk_ca55_0_ep_Rev100\pcie_xx_rzt2n_evk_ca55_0_ep.eww

       After changing the file path:
         C:\..\pcie_xx_rzt2n_evk_ca55_0_ep\pcie_xx_rzt2n_evk_ca55_0_ep.eww

  8. Start [Download and debug] for the EP Device first.
     The program will break at the first code in "main".

  9. Press [Go] to execute the EP Device program.
     Wait until the EP Device terminal menu is displayed.

  10. Start [Download and debug] for the RC Device on the second EWARM instance.

  11. Press [Go] to execute the RC Device program.
      Initial settings of cores and registers in SSLB and the sample application program
      will be run.


  - e²studio environment

  1. Prepare two e²studio instances using different workspaces.
     Connect USB Micro-B to J32, USB Micro-B to J33, and USB Type-C cable with AC adapter
     to J7 for both boards.
     Connect the two boards through J6 using the PCIe cable.
     Switch SW1 "POWER_SW" from OFF to ON on both boards.

  2. Start two e²studio instances.

  3. Import the EP and RC sample projects into different workspaces.

     - Select [File] -> [Import] -> General [Existing Projects into Workspace].
     - Check [select archive file].
     - Import:

         "pcie_ep_rzt2n_evk_ca55_0_ep_Rev100.zip"
         "pcie_rc_rzt2n_evk_ca55_0_ep_Rev100.zip"

  4. Generate the code in the Smart Configurator for both projects.
     Click [Generate Project Content] under "configuration.xml".

  5. Select [Build All] from the "Project" menu to rebuild both projects.

  6. Before starting debug, select the emulator that belongs to the target board in:

       [Debug Configurations] -> [Debugger] -> [Connection Settings]

     Start the EP Device first by selecting:

       [Debug As]->[pcie_ep_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)]

     When "Proceed with launch" is displayed, select [Yes].
     When "Confirm Perspective Switch" is displayed, select [Switch].

  7. Press [Resume].
     The program will break at "hal_entry();" in main.c.

  8. Press [Resume] again to execute the EP Device application.
     Wait until the menu is displayed on the EP Device terminal.

  9. On the second e²studio instance, start the RC Device by selecting:

       [Debug As]->[pcie_rc_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)]

     Select the emulator that belongs to the RC board in
     [Debug Configurations] -> [Debugger] -> [Connection Settings].

  10. Press [Resume].
      The program will break at "hal_entry();" in main.c.

  11. Press [Resume] again to execute the RC Device application.
