/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example projects demonstrates Message Queue and Semaphore between tasks and interrupt. Message Queue is 
    demonstrated between Tasks and between Task and interrupt. CMT timer periodically generates interrupt at 1000msec. 
    For first few seconds, messages are shared between Sender and Receiver Tasks and CMT msgq (Message Queue) ISR. 
    Receiver task pends on Message Queue, receives and displays message received on Tera Term periodically at 500msec. 
    For next few seconds, Semaphore Task waits for semaphore until it is released by CMT sem (semaphore) ISR. CMT sem 
    ISR releases semaphore periodically at 1000msec. All the tasks run with equal priority level.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.
    1 x USB Type-A to USB Mini-B cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow.
    Hardware Connection:

        - The operating mode of the RZ/T2H : xSPI1 boot mode (x1 boot serial flash)
          SW14.1 : ON
          SW14.2 : OFF
          SW14.3 : ON
          SW14.6 : OFF

        - CPU0 ATCM wait cycle = 1 wait cycle
          SW14.4 : OFF

        - Use UART data for USB-to-serial conversion
          SW4.5  : ON
          SW4.6  : OFF
          SW4.7  : ON
          SW4.8  : OFF

    Configuring the Evaluation board circuit
        - USB connection between Evaluation board and PC
          CN34 - USB Type-A to USB Mini-B cable
          CN14 - USB Type-A to micro USB cable