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
    RZ supported boards: RZ/N2L-RSK.
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

        2. *Enables signals other than the external bus. (CAN, Emulator, I2C, etc.).
        SW4.7  : ON

        3. *Set SW8 following.
        SW8.2  : ON
        SW8.4  : ON
        SW8.7  : ON
        SW8.10 : ON

        4. *Set SW11 following.
        SW11.1  : ON
        SW11.5  : ON
        SW11.7  : ON
        SW11.10 : ON
    
        5. Connect SDA pins and SCL pins between 2 channels

        |   J27 Grove (I2C)     |     CN1 header 20way     |
        |   (Channel 1: slave)  |   (Channel 0: master)    |
        |-----------------------|--------------------------|
        |  J27.1 |---SCL1  <----|---->  SCL0---|CN1.13|    |
        |  J27.2 |---SDA1  <----|---->  SDA0---|CN1.14|    |
        |-----------------------|--------------------------|

Note:
1. Segger RTT block address may be needed to download and observe EP operation using a hex file with RTT-Viewer.
   RTT Block address for hex file committed in repository are as follows:
   a. e2studio: 0x30100000.

2. If an EP is modified, compiled, and downloaded please find the block address (for the variable in RAM called _SEGGER_RTT) 
   in .map file generated in the build configuration folder (Debug/Release).