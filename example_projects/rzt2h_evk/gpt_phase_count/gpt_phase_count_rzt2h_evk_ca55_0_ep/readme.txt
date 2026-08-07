/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the use of the General Purpose PWM Timer (GPT) in phase counting mode 1 on the 
    RZ/T2H microprocessor. It counts pulses from a two-phase encoder (A phase, B phase) and provides Tera Term
    commands to get, set, and reset the count value. 

    On user input via Tera Term:
        "g": Retrieves the current count value from the GPT counter.
        "s": Allows the user to set a specific count value. After entering s, the user inputs the desired value.
        "r": Resets the GPT counter value to zero.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.
    1 x MB057GA140 Encoder (Motor).

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
        - The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
          SW14.1 : ON
          SW14.2 : OFF
          SW14.3 : ON
          SW14.6 : OFF

        - CPU0 ATCM wait cycle = 1 wait cycle
          SW14.4 : OFF

        - Use CN24-PIN5 as GPT (GPTIOC00_1A / P00_2)
          SW1-5 : OFF

        - Use UART data for USB-to-serial conversion
          SW4.5  : ON
          SW4.6  : OFF
          SW4.7  : ON
          SW4.8  : OFF

5. Hardware Connections:
    (1) Connect the CN14 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN34 port on evaluation board to the PC using USB Mini-B cable for serial terminal 
        communication (Tera Term).
    (3) Connect the CN46 port on evaluation board to a power source using a USB Type-C cable for board power supply.
    (4) Slide SW16 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
    (5) Connect the two-phase incremental encoder Phase A/B signals to the GPT using RS-485 conversion boards.

        |      CN24 header      |   MB057GA140(RS-485 Board)        |
        |-----------------------|-----------------------------------|
        |       CN24-PIN5  <----|---->      Phase A                 |
        |       CN24-PIN6  <----|---->      Phase B                 |
        |-----------------------|-----------------------------------|

    (6) Power supply and ground connections for RS‑485 boards:

        |     Evaluation Board  |   MB057GA140(RS-485 Board)        |
        |-----------------------|-----------------------------------|
        |  CN54‑PIN7 (5V)  <----|---->      +5V                     |
        |  CN54‑PIN8 (GND) <----|---->      GND                     |
        |-----------------------|-----------------------------------|

Note:
1. To echo back characters types in Tera Term, the user needs to enable it through
        [Setup]  -> [Terminal...] -> Check [Local echo]

2. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none

3. Encoder wiring via RS-485 conversion boards (MB057GA140):
   Phase A = GREEN wire: connect to RS-485 conversion board #1 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN24-PIN5.
   Phase B = YELLOW wire: connect to RS-485 conversion board #2 at Encoder CN4 (D+).
   From the same D+ terminal, route an additional wire to CN24-PIN6.

4. All RS‑485 conversion boards share a common 5V and GND supply.
   Power is sourced from the Evaluation Board via CN54 and distributed
   among the RS‑485 boards to ensure a common VCC and GND reference.