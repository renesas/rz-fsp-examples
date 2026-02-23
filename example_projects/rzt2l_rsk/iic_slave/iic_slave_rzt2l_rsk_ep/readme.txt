/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates typical use of the IIC slave HAL module APIs. The project initializes both the IIC slave 
    and IIC master module with standard rate and is made interfaced with loop-back mechanism. It performs Slave read and write 
    operation continuously once initialization is successful. 

    On successful I2C transaction (6 bytes), Data transceived is compared. LED blinks on data match else it is turned ON 
    as sign of failure. Output message for both corresponding slave operations is displayed on RTT Viewer.
    Any API/event failure message is also displayed.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.0.0
    e2 studio: Version 2025-12
    GCC ARM Embedded Toolchain: Version 13.3.1.arm-13-24
    SEGGER J-Link RTT Viewer: Version 8.60

3. Hardware Requirements:
    RZ supported boards: RZ/T2L-RSK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Type-A to micro USB cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI0 boot mode (x1 boot serial flash).
    (2) Set for DIP switches and jumpers as follow:
    Hardware Connection:

        1. *SW4 settings must be below to start this sample program.
        SW4.1 : ON
        SW4.2 : ON
        SW4.3 : ON

        2. *This sample program runs on TCM and CPU frequency is set to 800MHz in the start up process.
        Therefore, the following settings (MDW = 1) are required.
        SW4.4 : OFF

        3. *This sample program uses (I2C)
        Therefore, the following settings are required.
        SW8.10 : OFF

        4. *Configuring the RSK board circuit
        Connect SDA pins and SCL pins between 2 channels.
        |   J22 mikroBUS (I2C)  |    JA1-A header 26way    |
        |   (Channel 1: slave)  |   (Channel 0: master)    |
        |-----------------------|--------------------------|
        |  J22.5 |---SCL1  <----|---->  SCL0---|JA1-A.26|  |
        |  J22.6 |---SDA1  <----|---->  SDA0---|JA1-A.25|  |
        |-----------------------|--------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30080000

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).