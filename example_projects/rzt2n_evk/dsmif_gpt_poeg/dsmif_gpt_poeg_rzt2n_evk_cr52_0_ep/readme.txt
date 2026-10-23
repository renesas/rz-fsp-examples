File Name   : readme.txt
Description : Readme of the RZ/T2N DSMIF / GPT / POEG example project
              (dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep)


1. Project Overview:

    This example project demonstrates the ΔΣ interface (DSMIF) of the RZ/T2N together with the
    general PWM timer (GPT) and the port output enable for GPT (POEG), on the Renesas RZ/T2N
    Evaluation board.

    RZ/T2N microcontrollers have 5 units of DSMIF with 3 channels (U, V, W) per unit, for a
    total of 15 channels. Delta-sigma modulated 1-bit digital input data can be filtered and
    converted to 16-bit digital data. This sample program uses DSMIF Unit4 ch0.

    The software monitors the delta-sigma modulated current values through terminal software on
    a host PC. It also outputs a complementary PWM by the general PWM timer, and reads the
    converted current value with a PWM overflow interrupt. Overcurrent abnormality detection
    errors and short-circuit abnormality detection errors are monitored, and when one is
    detected by the DSMIF the PWM output is cut off by the POEG.

    Peripheral functions and applications.

      | Peripheral Function              | Application                                        |
      |----------------------------------|----------------------------------------------------|
      | Serial communication interface   | For communication with terminal software           |
      | (SCI)                            |                                                    |
      | ΔΣ Interface (DSMIF Unit4 ch0)   | Filter 1-bit digital data from an external         |
      |                                  | delta-sigma modulator and convert this into 16-bit |
      |                                  | data.                                              |
      | General PWM timer (GPT)          | Use for complementary PWM output of GPT1 ch0 to    |
      |                                  | ch2                                                |
      | Interrupt controller (ICU)       | Over current detection, short circuit detection,   |
      |                                  | GPT overflow interrupt and SCI transmit/receive    |
      |                                  | interrupt are used                                 |
      | Port Output Enable for GPT(POEG) | When overcurrent detection or short circuit        |
      |                                  | detection is detected by DSMIF, the PWM output is  |
      |                                  | cut off.                                           |

    Pins and pin functions.

      | Pin Name   | I/O    | Function                                                        |
      |------------|--------|-----------------------------------------------------------------|
      | MCLK40     | Input  | Clock I/O pin                                                   |
      | MDAT40     | Input  | Data input pin                                                  |
      | GTIOC01_0A | Output | GPT1 Channel0 output                                            |
      | GTIOC01_0B | Output | GPT1 Channel0' output (inverted-phase waveform of Channel0)     |
      | GTIOC01_1A | Output | GPT1 Channel1 output                                            |
      | GTIOC01_1B | Output | GPT1 Channel1' output (inverted-phase waveform of Channel1)     |
      | GTIOC01_2A | Output | GPT1 Channel2 output                                            |
      | GTIOC01_2B | Output | GPT1 Channel2' output (inverted-phase waveform of Channel2)     |

    Interrupts used by the sample program.

      | Interrupt                                     | Priority | Description                |
      |-----------------------------------------------|----------|----------------------------|
      | Interrupt Controller (ICU)(DSMIF_ERR0)        | 12       | DSMIF error detection.     |
      |                                               |          | Scan operation is stopped  |
      |                                               |          | on over current detection  |
      |                                               |          | and short circuit          |
      |                                               |          | detection.                 |
      | GPT1 GTCNT overflow interrupt of GPT          | 12       | Read capture current data  |
      |                                               |          | by GPT1 GTCNT overflow     |
      |                                               |          | interrupt.                 |
      | Serial Communications Interface_ch0 (SCI0)    | 12       | Key input detection        |

    DSMIF driver functions used by the sample program.

      | Function               | Description                          |
      |------------------------|--------------------------------------|
      | R_DSMIF_Open           | DSMIF open function.                 |
      | R_DSMIF_ScanStart      | DSMIF start function.                |
      | R_DSMIF_ScanStop       | DSMIF stop function.                 |
      | R_DSMIF_CfgSet         | DSMIF configuration function.        |
      | R_DSMIF_StatusGet      | DSMIF status getting function.       |
      | R_DSMIF_ErrorStatusGet | DSMIF error status getting function. |
      | R_DSMIF_Read           | DSMIF current data reading function. |
      | R_DSMIF_Close          | DSMIF close function.                |
      | R_DSMIF_CallbackSet    | DSMIF callback function              |

    The sample program menu has two items.

      [1] Start Measurement
          The current value data converted to 16 bits by filtering is measured 100 times.
          The capture current data is read by the overflow interrupt of GPT1.

      [2] Start Overcurrent/Short circuit Error Detection Measurement
          Error detection (overcurrent lower limit / upper limit detection error, short circuit
          error detection error) is performed. When an error is detected, the GPT output is cut
          off and the interrupt detection finishes.
          To output the current data again, restart the sample program.


2. Software Requirements:

    - IAR Embedded Workbench for ARM : 9.60.3 + patch
      Smart Configurator + RZ_FSP_Packs_v4.2.0

    - e²studio_2026-07 with RZ FSP
      Toolchain           : GNU ARM Embedded (Version:13.3.1.arm-13-24)
      RZ FSP version   : 4.2.0
      J-Link Base         : Ver.11.0

    - Terminal Emulator (Tera Term v5.40)


3. Hardware Requirements:

    - Target board      : RZ/T2N Evaluation board  x 1
    - MPU               : RZ/T2N Group
    - Sub-board         : RV1S9355A (external delta-sigma modulator)
    - Target Core       : CPU Core0 (Arm Cortex-R52), 800 MHz
    - Operating voltage : 3.3V / 1.8V / 1.1V
    - Boot mode         : RAM execution

    - 1 x USB Micro-B cable (for J-Link OB)
    - 1 x USB Micro-B cable (for serial communication)
    - 1 x IAR I-jet (for debugging in IAR EWARM)
    - 1 x USB Type-C cable supporting power delivery(PD) and 15V/3A DC (for power supply)
    - 1 x AC adapter for USB supporting 15V/3A
    - 1 x stabilized power supply with 2 output channels (for the RV1S9355A sub-board)
    - 1 x oscilloscope with 2 or more channels (for observing the complementary PWM output)


4. Hardware Configurations:

    Configure the RZ/T2N Evaluation board by switches and jumper blocks as below.

    - The operating mode of the RZ/T2N : xSPI1 boot mode (x1 boot serial flash)
        SW8.1   : ON
        SW8.2   : OFF
        SW8.3   : ON

    - 2nd Boot CPU : Cortex-R52 CPU0
        SW10.10 : ON
        SW10.7  : OFF

    - SW7
        SW7.1   : OFF


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

    Connection points on the Evaluation board.

      | Function       | Connector   | Pin | Signal     | IOPORT |
      |----------------|-------------|-----|------------|--------|
      | DSMIF (Unit 4) | J55         | 4   | MCLK40     | P12_5  |
      |                |             | 3   | MDAT40     | P12_6  |
      | GPT            | INV3 (J51)  | 28  | GTIOC01_0A | P03_6  |
      |                |             | 29  | GTIOC01_0B | P03_7  |
      |                |             | 30  | GTIOC01_1A | P04_1  |
      |                |             | 31  | GTIOC01_1B | P04_0  |
      |                |             | 32  | GTIOC01_2A | P04_2  |
      |                |             | 33  | GTIOC01_2B | P04_3  |

    (1) RV1S9355A sub-board

    Connect the RV1S9355A sub-board to the J55 connector of the Evaluation board. MCLK, MDAT,
    3.3 V (VDD2) and GND are supplied through J55.

        RZ/T2N J55                        RV1S9355A sub-board
        J55-4  MCLK40 (P12_5)  ---------  CLK
        J55-3  MDAT40 (P12_6)  ---------  DAT
        3.3 V (VDD2)           ---------  VDD2
        GND                    ---------  GND

    The external supply side of the RV1S9355A uses VDD1 (5 V), Vin+, Vin- and GND. Connect a
    stabilized power supply with two output channels as below.

        Channel A : 5.0 V, current limit 0.2 A
          +  ---> RV1S9355A VDD1 (5 V)
          -  ---> RV1S9355A GND

        Channel B : 0.2 V, current limit 0.2 A
          +  ---> RV1S9355A Vin+
          -  ---> RV1S9355A Vin-

    The differential input is therefore Vin+ - Vin- = about +0.2 V.
    NEVER connect 5 V to Vin+ / Vin-. The 5 V output is only for VDD1.

    (2) Oscilloscope for the complementary PWM output

    It is not necessary to observe all six PWM pins at the same time. One complementary pair is
    enough to check the operation. Connect the probes to the pair of GPT channel 0 first.

        Channel A signal --- J51-28  (GTIOC01_0A / P03_6)      pair 0
        Channel B signal --- J51-29  (GTIOC01_0B / P03_7)
        Probe GND        --- a verified GND point of the board

    To check the other pairs, move only the two signal leads.

        pair 1 : channel A signal --- J51-30, channel B signal --- J51-31
        pair 2 : channel A signal --- J51-32, channel B signal --- J51-33

    Note that J51-30 to J51-33 are PWM output pins. Never connect a ground lead to them.

    (3) Power-up order

    Connect everything while all the power is OFF, and then turn the power on in this order.

        1. Connect the RV1S9355A sub-board to J55 of the Evaluation board.
        2. Connect the stabilized power supply wires to the RV1S9355A.
        3. Connect the oscilloscope probes.
        4. Connect J32 (or J31) and J33 to the host PC.
        5. Turn on the power of the Evaluation board with SW1 "POWER_SW".
        6. Turn on the 5.0 V output for the RV1S9355A VDD1.
        7. Turn on the 0.2 V output for Vin+ / Vin-.

    Turn on the power of the Evaluation board FIRST, and the stabilized power supply after it.


Note:

    - This sample program displays the results on a PC (terminal software) via a COM port.
      Set the configuration of Terminal Emulator (Tera Term) as below.

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

      Set the reception line feed code of the terminal software to [AUTO].

    - How to convert from the A/D conversion value to a voltage value.

        When using a ΔΣ modulator (RV1S9355A) and current data 32768 (= 0 mV) or more,
          Voltage [mV] = (current data - 0x8000) * 320 [mV] / 0x7FFF

        When current data is 32768 (= 0 mV) or less,
          Voltage [mV] = (current data - 0x8000) * 320 [mV] / 0x8000

        Ex) When current data is 53606,
            203.521836 mV (= (53606 - 32768 (= 0x8000)) * 320 [mV] / 32767 (0x7FFF))

      With about +0.2 V applied to the modulator, values in the 53xxx region are expected.
      They do not have to be exactly 53606.

    - After an error is detected with the menu item [2], the interrupt detection finishes and
      the program ends. To output the current data again, restart the sample program.

    - Before starting debug, please push the reset button on the board to reset RZ/T2N.

    - Since this sample program executes RAM, it is prohibited to press the "Reset" button
      during debugging.

    - (e²studio environment only)
      After building the project, the build completes normally, but there are times when warning
      markers such as "Cannot resolve symbol '****'" do not disappear.
      In that case, there is no problem in debugging as it is.
      If you want to remove the warning mark, after right-clicking
      "dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep" in Project Explorer, it can be deleted by
      [Index] > [Rebuild].


Start Debugging:

  - EWARM environment

  1. Connect IAR I-jet to J31 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Complete the connections of "5. Hardware Connections" and turn on the power in the order
     described there.

  2. Unzip [iar]-[dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep.zip] to any folder.

  3. Start IAR EWARM.

  4. Select [File]-[Open Workspace] from the EWARM menu, 
     and open "dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep\dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep.eww"
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

  8. Select [Project]-[Download and Debug] from the EWARM menu.
     The program will break at the first code in "main".

  9. Select [Debug]-[Go] from the EWARM menu to run the application program.


  - e²studio environment

  1. Connect USB Micro-B to J32 and USB Type-C cable with AC adapter to J7.
     Also, connect USB Micro-B to J33, set a terminal soft on PC.
     Complete the connections of "5. Hardware Connections" and turn on the power in the order
     described there.

  2. Start e²studio.

  3. Import the sample project.
    - Select [File] -> [Import] -> General [Existing Projects into Workspace] from e²studio toolbar.
    - Check the [select archive file] and select "dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep.zip",
      and push [Finish].

  4. Generate the code in the Smart Configurator.
     Click [Generate Project Content] under "configuration.xml".

  5. Select the [Build All] item from the "Project" menu to rebuild the project.

  6. In [Project Explorer] view,
     right click the node of the project to be debugged and select
     [Debug As]->[dsmif_gpt_poeg_rzt2n_evk_cr52_0_ep Debug_Flat
     (Renesas DBG Hardware Debugging)] menu.

     When "Proceed with launch" window is displayed, select [Yes].

     When "Confirm Perspective Switch" window is displayed, select [Switch].

  7. Press the [Resume] button.
     The program will break at "hal_entry();"in main.c .

  8. Press the [Resume] button again to execute the application program.


Execution of sample program:

  After running the application program, the following menu is displayed on the terminal
  software, and the complementary PWM is already output on J51-28 / J51-29.

      RZ/T2N DSMIF Sample Program
      [1] Start Measurement
      [2] Start Overcurrent/Short circuit Error Detection Measurement
      >

  (1) Normal measurement

      With both stabilized power supply outputs ON, enter "1" and press Enter.
      100 converted values are printed, 10 values per line. With about +0.2 V applied, values in
      the 53xxx region are expected. Confirm that the values are stable, that they are not all
      0 and not all 65535, and that the complementary PWM continues on the oscilloscope.

  (2) Short circuit detection

      Enter "2" and press Enter to go into the OverCurrent / Short circuit Detection Mode, and
      then turn OFF the stabilized power supply.

          Channel 0 short circuit detection
          Error Detection.
          Finish RZ/T2 DSMIF Sample Program

      At the same time the PWM output on the oscilloscope must stop and stay at a fixed level.

  (3) Overcurrent lower limit detection

      Stop debugging and turn OFF the stabilized power supply. Start debugging again while the
      supply is still OFF, run the program, enter "2" and press Enter, and then turn ON the
      stabilized power supply.

          Channel 0 overcurrent lower limit detection
          Error Detection.
          Finish RZ/T2 DSMIF Sample Program

      The PWM output must be cut off as well.

  (4) Overcurrent upper limit detection

      Record the original Upper Limit value of the DSMIF stack first, then set the Upper Limit
      "Limit Value" to 50000 in the FSP configuration, generate the code and build again.
      Turn ON the stabilized power supply, start debugging, run the program, enter "2" and press
      Enter.

          Channel 0 overcurrent upper limit detection
          Error Detection.
          Finish RZ/T2 DSMIF Sample Program

      The PWM output must be cut off as well.
      After the test, restore the original Upper Limit value, generate the code and build again.
      Do not leave the value 50000 in the delivered project.