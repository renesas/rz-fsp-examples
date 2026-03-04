/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This Example Project demonstrates inter-CPU data exchange using the shared_memory module on RZ/T.
    CPU0 initializes the shared memory field, both CPUs open the driver (peer waits until READY_TO_WRITE).
    CPU0 writes the first half of shared memory and generates interrupt to CPU1.
    CPU1 reads it from the shared memory, writes data to the second half, and generates interrupt to CPU0.
    CPU0 reads back, verifies the expected pattern, reports MATCH/FAIL, and both close the driver.
    The result is displayed on JLink RTT Viewer at both CPU0 and CPU1 sides.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2M-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        16-bit bus boot mode (NOR flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *External flash memory must be blank to start this sample program (RAM execution).
        NOR flash memory on this board is blank, so set the operating mode switch for 16-bit bus boot mode (NOR flash).
        SW4.1 : ON
        SW4.2 : OFF
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.5 : OFF

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30180000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).
   
3. Step to debug the project in e2studio:
   Generate code and build both CPU0 and CPU1.
   Start a hardware debug session on CPU0 and let it break at system_init() then main()
   Keep CPU0 suspended. With CPU0 still connected, start debugging CPU1 (choose “No” when warned about an active GDB session)
   Switch to the CPU1 thread, Resume to hal_entry() and Resume again to run CPU1.
   Finally switch back to the CPU0 thread and Resume to run CPU0.
