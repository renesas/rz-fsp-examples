/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    This example project demonstrates typical use of the xSPI_QSPI driver with an external
    Serial NOR flash. Upon successful initialization, the project erases a memory sector.
    Issue Page Program (0x02) and Fast Read (0xEB) at a test address and verify the data.
    Read the same location via the memory-mapped region (direct read) and verify again.
    Finally, write a small data to buffer and confirm via memory-mapping read.
    Any API or event is printed to the Terminal Emulator.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/T2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Micro-B cable.
    1 x USB Mini-B cable.

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

        - Use UART data for USB-to-serial conversion
          SW4.5 : ON
          SW4.6 : OFF
          SW4.7 : ON
          SW4.8 : OFF

        - Use XSPI1
          SW1.6 : ON

        - Use UART data for USB-to-serial conversion
          SW4.5 : ON
          SW4.6 : OFF
          SW4.7 : ON
          SW4.8 : OFF

5. Hardware Connections:
    (1) Connect the CN14 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN34 port on evaluation board to the PC using USB Mini-B cable for serial terminal 
        communication (Tera Term).
    (3) Connect the CN46 port on evaluation board to a 15V power source using a USB Type-C cable for board power supply.
    (4) Slide SW16 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.

Note:
1. Do not set the XSPI1 clock speed (XSPI_CLK1) to 12.5MHz. The project may not work properly.

2. Chip Erase is a time-consuming process, so it is commented out when the example project is provided.
   How to use Chip Erase:
   (1) Comment out the "Sector Erase" code block in hal_entry() function.
   (2) Uncomment the "Chip Erase" code block in hal_entry() function.
   (3) Re-build the project.
   (4) Download EP to RZ board and run the project. Then confirm Chip Erase operation via the Terminal Emulator.