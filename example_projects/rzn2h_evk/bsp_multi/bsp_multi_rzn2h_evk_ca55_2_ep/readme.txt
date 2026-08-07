/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates the on-board LEDs using BSP APIs on multi-core CA55 Core0, CA55 Core1,
    CA55 Core2, CA55 Core3, CR52 CPU0, and CR52 CPU1. Once started, each core will toggle each LED periodically 
    using CMT timer. Any API/event failure message is displayed.

    The details of the operation flow for each core are as follows.
    --------------------------------------------------------------------------------------------
    | Core       | CA55 Core0 | CA55 Core1 | CA55 Core2 | CA55 Core3 | CR52 CPU0  | CR52 CPU1  |
    |------------|------------|------------|------------|------------|------------|------------|
    | CMT n[ch]  | 0          | 1          | 2          | 3          | 4          | 5          |
    |------------|------------|------------|------------|------------|------------|------------|
    | LEDn       | LED3       | LED4       | LED5       | LED6       | LED7       | LED8       |
    --------------------------------------------------------------------------------------------

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Micro-B cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).

    (2) Target Core: Cortex-A55 Core2.

    (3) Set for DIP switches and jumpers as follow.
        - Boot mode switch:
          DSW3.1 : ON
          DSW3.2 : OFF
          DSW3.3 : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4 : OFF

        - Use user LEDs
          DSW15.8 : OFF
          DSW15.9 : OFF
          DSW15.10: ON

          DSW18.1 : ON
          DSW18.2 : OFF
          DSW18.3 : ON
          DSW18.4 : OFF
          DSW18.5 : ON
          DSW18.6 : OFF
          DSW18.7 : ON
          DSW18.8 : OFF
          DSW18.9 : ON
          DSW18.10: OFF

          DSW19.3 : OFF
          DSW19.4 : ON

        - Use UART data for USB-to-serial conversion
          DSW9.1 : ON
          DSW9.2 : OFF
          DSW9.3 : ON
          DSW9.4 : OFF

5. Hardware Connections:
    (1) Connect the CN26 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN13 port on evaluation board to a 15V power source using a USB Type-C cable for board power supply.
    (3) Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.

6. Limitations:
    When using J-link OB on the EVK board to debug on e2 studio, the number of cores for multi-core debugging is limited to 5 cores.

Note:
1. To debug project on IDE, should import all projects and build projects in the following order:
    [1] bsp_multi_rzn2h_evk_ca55_0_ep
    [2] bsp_multi_rzn2h_evk_ca55_1_ep
    [3] bsp_multi_rzn2h_evk_ca55_2_ep
    [4] bsp_multi_rzn2h_evk_ca55_3_ep
    [5] bsp_multi_rzn2h_evk_cr52_0_ep
    [6] bsp_multi_rzn2h_evk_cr52_1_ep

2. Follow steps 1 to 3 to start debugging the project and run the example project in the following order:
    [1] bsp_multi_rzn2h_evk_ca55_0_ep
    [2] bsp_multi_rzn2h_evk_ca55_1_ep
    [3] bsp_multi_rzn2h_evk_ca55_2_ep
    [4] bsp_multi_rzn2h_evk_ca55_3_ep
    [5] bsp_multi_rzn2h_evk_cr52_0_ep
    [6] bsp_multi_rzn2h_evk_cr52_1_ep

    Step 1. Right-click the project in the [Project Explorer] view and select [Debug] > [Renesas DBG Hardware Debugging].
        After starting to debug the project, the program breaks at the first code of "system_init" in "startup_core.c".
        - If the message "A Renesas GDB debug session is already active" appears, select [No].
        - If the message "Proceed with launch?" appears, select [Yes].

    Step 2. Click the [Resume] button to restart debugging.
        In the project, the program breaks at "hal_entry()" in "main.c".

    Step 3. Press the "Resume" button again to execute the application program.


