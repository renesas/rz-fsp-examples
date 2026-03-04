/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example projects demonstrates Message Queue and Semaphore between tasks and interrupt.
    Message Queue is demonstrated between Tasks and between Task and interrupt. GTM timer
    periodically generates interrupt at 1000 miliseconds. For first few seconds, messages are
    shared between Sender and Receiver Tasks and GTM msgq (Message Queue) ISR. Receiver task
    pends on Message Queue, receives and displays message received on RTT Viewer periodically
    at 500 miliseconds. For next few seconds, Semaphore Task waits for semaphore until it is
    released by GTM sem (semaphore) ISR. GTM sem ISR releases semaphore periodically at 1000
    miliseconds. All the tasks run with equal priority level.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/V2L-EVK.
    1 x RZ board.
    1 x USB Type-C cable.

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).
    (2) Set for DIP switches and jumpers as follow.

        +================+================================+
        | Board setting  | RZ/V2L Evaluation Kit (SMARC)  |
        +================+================================+
        | Module board   | SW1-1  : OFF                   |
        |                | SW1-2  : Don't care            |
        +----------------+--------------------------------+
        | Carrier board  | SW1    : Don't care            |
        |                | SW2    : Don't care            |
        |                | SW3    : Don't care            |
        |                | SW4    : Don't care            |
        |                | SW5    : Don't care            |
        |                | SW6    : Don't care            |
        |                | SW7    : Don't care            |
        |                | SW8    : Don't care            |
        |                | SW11-1 : OFF                   |
        |                | SW11-2 : OFF                   |
        |                | SW11-3 : OFF                   |
        |                | SW11-4 : ON                    |
        |                |                                |
        |                | CN4    : Jumper connects 1-3   |
        |                |          Jumper connects 2-4   |
        +----------------+--------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable for board power
        supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.

Note:
1. Segger RTT block address may be needed to download and observe example project operation
   using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x60110470

2. If an example project is modified, compiled, and downloaded please find the block address
   (for the variable in RAM called _SEGGER_RTT) in .map file generated in the build
   configuration folder (Debug/Release).