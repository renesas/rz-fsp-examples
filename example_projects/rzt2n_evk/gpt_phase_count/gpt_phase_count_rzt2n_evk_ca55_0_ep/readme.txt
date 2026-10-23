File Name   : readme.txt
Description : Readme of the RZ/T2N GPT phase count example project
              (gpt_phase_count_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project uses the phase counting mode (phase counting mode 1) function of the
    general purpose PWM timer (GPT) of the RZ/T2N, and counts the number of pulses of a
    two-phase encoder (A phase, B phase) on the Renesas RZ/T2N Evaluation board.

    The main function of the program is listed below.
      - Command input from the terminal acquires the count value of the two-phase encoder
        (A phase, B phase) pulse number, sets the count value, and resets the count value.

    Peripheral functions and applications.

      | Peripheral Function                     | Application                                  |
      |-----------------------------------------|----------------------------------------------|
      | Serial communication interface (SCI)    | Used for setting instructions from the       |
      |                                         | terminal. (Get, Set or Reset instructions)   |
      | General PWM Timer (GPT)                 | Counts the number of pulses of a two-phase   |
      |                                         | encoder (A phase, B phase).                  |

    Pins and functions.

      | Pin Name                                      | I/O    | Function                    |
      |-----------------------------------------------|--------|-----------------------------|
      | TXD1(P11_2)                                   | Output | Send data to terminal       |
      | RXD1(P11_1)                                   | Input  | Receive data from terminal  |
      | P00_3_V1833_2_BSC_A19_INV1_GTIOC00_1A (P00_3) | Input  | Two-phase encoder Phase A   |
      |                                               |        | signal                      |
      | P00_2_V1833_2_BSC_A18_INV1_GTIOC00_1B (P00_2) | Input  | Two-phase encoder Phase B   |
      |                                               |        | signal                      |

    Constants of the sample program.

      | Constant Name         | Setting Value | Description                                     |
      |-----------------------|---------------|-------------------------------------------------|
      | CHARACTER_LENGTH_BYTE | 20            | Maximum data length of character string (Byte)  |
      | DATA_LENGTH_BYTE      | 4             | Maximum data length that can be entered (Byte)  |

    Application functions of the sample program.

      | Layer / Block | Function Name           |
      |---------------|-------------------------|
      | Application   | hal_entry ()            |
      |               | initialize ()           |
      |               | input_data_check ()     |
      |               | convert_char_to_hex ()  |
      |               | user_uart_callback ()   |
      |               | handle_error ()         |

    SCI module functions used by the sample program.

      | Function                 | Description                                              |
      |--------------------------|----------------------------------------------------------|
      | R_SCI_UART_Open          | SCI_UART open function                                   |
      | R_SCI_UART_Close         | SCI_UART close function                                  |
      | R_SCI_UART_Read          | Read from UART device                                    |
      | R_SCI_UART_Write         | Write to UART device                                     |
      | R_SCI_UART_CallbackSet   | User callback function                                   |
      | R_SCI_UART_BaudSet       | Update SCI_UART baud rate                                |
      | R_SCI_UART_InfoGet       | Provides driver information                              |
      | R_SCI_UART_Abort         | Provides an API to abort an in-progress transfer         |
      | R_SCI_UART_ReadStop      | Provides an API to abort a read in progress              |
      | R_SCI_UART_BaudCalculate | Calculate the set value of the baud rate register        |

    GPT module functions used by the sample program.

      | Function              | Description                                                 |
      |-----------------------|-------------------------------------------------------------|
      | R_GPT_Open            | GPT module open function                                    |
      | R_GPT_Stop            | Stop the GPT module                                         |
      | R_GPT_Start           | Start the GPT module                                        |
      | R_GPT_Reset           | Reset the GPT Phase Count timer                             |
      | R_GPT_Enable          | Enables external event triggers.                            |
      | R_GPT_Disable         | Disables external event triggers.                           |
      | R_GPT_PeriodSet       | Sets period value provided.                                 |
      | R_GPT_DutyCycleSet    | Sets duty cycle on requested pin.                           |
      | R_GPT_CompareMatchSet | Sets the value of the compare match function                |
      | R_GPT_InfoGet         | Get information such as GPT timer cycle settings.           |
      | R_GPT_StatusGet       | Get the status of GPT timer such as timer count value.      |

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as main().
      2. R_SCI_UART_Open() opens the SCI UART function.
      3. R_GPT_Open() opens the GPT function.
      4. R_GPT_Start() starts the GPT phase count.
      5. The sample program menu is displayed on the terminal software and the program waits
         for a command.
      6. When "g" is entered, R_GPT_StatusGet() reads the GPT counter value and it is printed
         on the terminal.
      7. When "s" is entered, the program transitions to setting mode and the counter value
         entered on the terminal is set.
      8. When "r" is entered, R_GPT_Reset() clears the GPT counter value.
      9. The program returns to step 5.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GCC ARM A-Profile(AArch64 bare-metal) (Version:13.2.1.arm-13-7)
      RZ FSP version   : 4.2.0

    - Terminal Emulator (Tera Term v5.40)


3. Hardware Requirements:

    - Target board      : RZ/T2N Evaluation board  x 1
    - MPU               : RZ/T2N Group
    - Target Core       : CPU Core0 (Arm Cortex-A55), 1200 MHz
    - Operating voltage : 3.3V / 1.8V / 1.1V
    - Boot mode         : RAM execution
    - Encoder (Motor)   : MB057GA140
    - Conversion board  : RS-485 board

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x SEGGER J-Link Base Ver.11.0 (for debugging in e²studio)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - SW8
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - SW10
        SW10.2  : ON
        SW10.3  : ON
        SW10.7  : OFF
        SW10.10 : OFF

    - SW7
        SW7.3   : OFF

    - Jumper
        J15     : 1-2 short
        J18     : 1-2 short


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

    Connect the two-phase encoder to the J49 pin header of the Evaluation board through the
    RS-485 conversion board.

      | Function | Connector | Pin                | Signal                                |
      |----------|-----------|--------------------|---------------------------------------|
      | SCI      | J33       | RXD1(P11_1)        | P11_1_FT2232_RXD1                     |
      |          |           | TXD1(P11_2)        | P11_2_FT2232_TXD1                     |
      | GPT      | J49-30    | GTIOC00_1A(P00_3)  | P00_3_V1833_2_BSC_A19_INV1_GTIOC00_1A |
      |          | J49-31    | GTIOC00_1B(P00_2)  | P00_2_V1833_2_BSC_A18_INV1_GTIOC00_1B |

    The SCI signals are wired to the USB connector on the RZ/T2N Evaluation board, and the GPT
    signals are wired to the pin header on the RZ/T2N Evaluation board.

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


Note:

    - This sample program communicates with the terminal software on the host PC through a COM
      port. Set the configuration of Terminal Emulator (Tera Term) as below.

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

    - Enter the commands of the sample program in lower case only.

    - For the Count Set operation, if you enter 9 or more characters for the data, the 8
      characters counted from the beginning are used for writing.
      e.g. for "Data: 123456789", the data is "12345678".

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "gpt_phase_count_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[gpt_phase_count_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "gpt_phase_count_rzt2n_evk_ca55_0_ep\gpt_phase_count_rzt2n_evk_ca55_0_ep.eww"
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
    - Check the [select archive file] and select "gpt_phase_count_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[gpt_phase_count_rzt2n_evk_ca55_0_ep Debug_Flat
     (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.