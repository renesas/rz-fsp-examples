File Name   : readme.txt
Description : Readme of the RZ/T2N MTU3 complementary PWM / POE3 example project
              (mtu3_poe3_rzt2n_evk_ca55_0_ep)


1. Project Overview:

    This example project outputs a three-phase (positive and negative, in total of 6 phases)
    dead-time PWM waveform by using the complementary PWM mode of the multi-function timer
    pulse unit3 (MTU3), MTU3 and MTU4, on the Renesas RZ/T2N Evaluation board.
    The port output enable3 (POE3) is used to cut off the complementary PWM output.

    The major features of the program are listed below.
      - Output of complementary PWM waveform with the cycle of the carrier (100 us) by using
        MTU3 and MTU4 and dead time (2 us).
      - Switching of PWM duty ratio by every press-down of Usr_SW2 (SW4) to 25%, 50%,
        and 75% (repeatedly).
      - When Usr_SW1 (SW3) is pushed, POE3 cuts off the complementary PWM output and
        the sample program is finished.

    Peripheral functions and applications.

      | Peripheral Function                     | Application                                  |
      |-----------------------------------------|----------------------------------------------|
      | Multi-function timer pulse unit3 (MTU3) | The MTU is used for complementary PWM output |
      |                                         | of the MTU3 ch3 and MTU3 ch4                 |
      | Port output enable3 (POE3)              | Cut off the complementary PWM output         |

    Functional overview of the sample program.

      | Function       | Description                                                          |
      |----------------|----------------------------------------------------------------------|
      | Channel        | MTU3 Channel 3 (MTU3), MTU3 channel 4 (MTU4)                         |
      | PWM output     | Three phases for positive and negative (in total of 6),              |
      |                | cycle of carrier (100 us), dead time (2 us)                          |
      | Operating mode | Complementary PWM mode 1 (transferred at crest)                      |
      | Clock          | PCLKH/4 (= 50.0 MHz), rising edge                                    |
      | Duty ratio     | 25%, 50%, and 75% common to all phases                               |

    Pins and pin functions.

      | Pin Name | I/O    | Function                                                          |
      |----------|--------|-------------------------------------------------------------------|
      | MTIOC3B  | Output | PWM Output 1                                                      |
      | MTIOC3D  | Output | PWM Output 1' (inverted-phase waveform output of PWM output 1)    |
      | MTIOC4A  | Output | PWM Output 2                                                      |
      | MTIOC4C  | Output | PWM Output 2' (inverted-phase waveform output of PWM output 2)    |
      | MTIOC4B  | Output | PWM Output 3                                                      |
      | MTIOC4D  | Output | PWM Output 3' (inverted-phase waveform output of PWM output 3)    |

    MTU3 driver functions used by the sample program.

      | Function                        | Description                                          |
      |---------------------------------|------------------------------------------------------|
      | R_MTU3_THREE_PHASE_Open         | Initializes the complementary PWM module (and         |
      |                                 | associated timers) and applies configurations.        |
      | R_MTU3_THREE_PHASE_Stop         | Stops all timers synchronously.                       |
      | R_MTU3_THREE_PHASE_Start        | Starts all timers synchronously.                      |
      | R_MTU3_THREE_PHASE_Reset        | Resets the counter values to 0.                       |
      | R_MTU3_THREE_PHASE_DutyCycleSet | Sets duty cycle for all three timers.                 |
      | R_MTU3_THREE_PHASE_CallbackSet  | Updates the user callback for the MTU3 U-channel with |
      |                                 | the option to provide memory for the callback         |
      |                                 | argument structure.                                   |
      | R_MTU3_THREE_PHASE_Close        | Stops counters, disables output pins, and clears      |
      |                                 | internal driver data.                                 |

    POE3 driver functions used by the sample program.

      | Function             | Description               |
      |----------------------|---------------------------|
      | R_POE3_Open          | POE3 Open function        |
      | R_POE3_OutputDisable | Disables MTU3 output pins |
      | R_POE3_Reset         | Resets the status flags   |
      | R_POE3_StatusGet     | Get current POE3 status   |
      | R_POE3_Close         | POE3 Close function       |

    The details of the operation flow are shown below.
      1. Start the debugging and sample program as main().
      2. R_IOPORT_Open() opens the I/O port.
      3. R_MTU3_THREE_PHASE_Open() opens the complementary PWM for the MTU3 ch3 and ch4.
      4. R_POE3_Open() opens the POE3.
      5. R_MTU3_THREE_PHASE_Start() starts the complementary PWM.
      6. While Usr_SW1 (SW3) is not pushed, the switch inputs are polled in the main
         loop in hal_entry(). Every press-down of Usr_SW2 (SW4) changes the duty ratio
         (25% -> 50% -> 75% -> 25% -> repeatedly). The buffer registers MTU3.TGRD,
         MTU4.TGRC and MTU4.TGRA are updated by R_MTU3_THREE_PHASE_DutyCycleSet().
      7. When Usr_SW1 (SW3) is pushed, R_POE3_OutputDisable() changes the
         complementary PWM output to Hi-z.
      8. R_MTU3_THREE_PHASE_Stop() stops the complementary PWM for the MTU3 ch3 and ch4.
      9. R_MTU3_THREE_PHASE_Close() closes the complementary PWM for the MTU3 ch3 and ch4.
     10. R_POE3_Close() closes the POE3, and the sample program is finished.


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

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x SEGGER J-Link Base Ver.11.0 (for debugging in e²studio)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A
    - 1 x oscilloscope or logic analyzer with 2 or more channels
      (for observing one complementary pair of the PWM waveform on the J50 connector at the
       same time)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-A55 Core0
        SW10.10 : OFF
        SW10.7  : OFF

    - SW11
        SW11.5  : OFF
        SW11.6  : OFF
        SW11.7  : ON


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

    The complementary PWM waveform of this sample program is output from the J50 connector.
    Connect an oscilloscope or a logic analyzer to the following pins to observe it.

      | Function | Connector     | Pin | Signal  | IOPORT | Complementary pair |
      |----------|---------------|-----|---------|--------|--------------------|
      | MTU3 ch3 | J50 Connector | 28  | MTIOC3B | P14_1  | pair 1 (normal)    |
      | MTU3 ch4 |               | 29  | MTIOC3D | P14_2  | pair 1 (inverted)  |
      |          |               | 30  | MTIOC4A | P14_4  | pair 2 (normal)    |
      |          |               | 31  | MTIOC4C | P14_5  | pair 2 (inverted)  |
      |          |               | 32  | MTIOC4B | P14_6  | pair 3 (normal)    |
      |          |               | 33  | MTIOC4D | P14_7  | pair 3 (inverted)  |

    J50-27 and J50-34 are GND. Use them for the ground leads of the probes.

    A pair of channels is enough to check the operation. Connect the probes as below, and
    change only the two signal leads to check the other pairs. The ground leads can stay
    where they are.

        Channel A signal --- J50-28  (MTIOC3B / P14_1)      pair 1
        Channel A GND    --- J50-27  (GND)
        Channel B signal --- J50-29  (MTIOC3D / P14_2)
        Channel B GND    --- J50-34  (GND)

        pair 2 : channel A signal --- J50-30, channel B signal --- J50-31
        pair 3 : channel A signal --- J50-32, channel B signal --- J50-33

    Note that J50-28 to J50-33 are signal pins. Never connect a ground lead to them.
    Both ground leads may also be connected to the same GND pin if the connector allows it.

    The user switches Usr_SW1 and Usr_SW2 on the Evaluation board are used by this sample
    program.
    Usr_SW1 / Usr_SW2 are printed on the board next to the push buttons; SW3 / SW4 are
    their reference designators (also the pin names in the FSP configuration).
      - Usr_SW2 (SW4) : changes the PWM duty ratio
        (25% -> 50% -> 75% -> 25% -> repeatedly)
      - Usr_SW1 (SW3) : cuts off the complementary PWM output by POE3 and finishes the
        sample program

    After all the connections are completed, switch SW1 "POWER_SW" from OFF to ON to turn on
    the power supply.


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

    - The following MTU output pins have the same output.
        MTIOC4A (normal) and MTIOC4C (inverted)
        MTIOC4B (normal) and MTIOC4D (inverted)

    - Expected waveform of a complementary pair.
      While the normal-phase pin is HIGH the inverted-phase pin is LOW, and vice versa.
      Both pins are OFF during the dead time inserted at every change of the state.
      The cycle of the carrier is 100 us (about 10 kHz) and the dead time is 2 us.

        normal   ----+--------+        +--------+
                     |        |        |        |
                     +        +--------+        +--------
        inverted ----+        +--------+        +--------
                     |        |        |        |
                     +--------+        +--------+
                              ^
                          dead time

    - After Usr_SW1 (SW3) is pushed, POE3 changes the output pins to Hi-z. The pins are no longer
      driven, so the level observed on the oscilloscope is not necessarily 0 V and may float
      depending on the conditions. What has to be confirmed is that the PWM waveform is no
      longer output.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "mtu3_poe3_rzt2n_evk_ca55_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33 and set terminal software on the PC.
     Switch SW1 "POWER_SW" from OFF to ON to turn on the power supply.

  2. Unzip [iar]-[mtu3_poe3_rzt2n_evk_ca55_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "mtu3_poe3_rzt2n_evk_ca55_0_ep\mtu3_poe3_rzt2n_evk_ca55_0_ep.eww" in the folder
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
    - Check the [select archive file] and select "mtu3_poe3_rzt2n_evk_ca55_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[mtu3_poe3_rzt2n_evk_ca55_0_ep Debug_Flat (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.