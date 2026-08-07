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
    Any API or event is printed to the Tera Term.

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    RZ supported boards: RZ/N2H-EVK.
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Micro-B cable.
    1 x USB Mini-B cable.

4. Hardware Configurations:
    (1) Boot Mode:
        xSPI1 boot mode (x1 boot serial flash).
    (2) Set for DIP switches as follow.
        - The operating mode of the RZ/N2H : xSPI1 boot mode (x1 boot serial flash)
          DSW3.1 : ON
          DSW3.2 : OFF
          DSW3.3 : ON

        - CPU0 ATCM wait cycle = 1 wait cycle
          DSW3.4 : OFF

        - Use XSPI1
          DSW2.6 : ON

        - Use UART data for USB-to-serial conversion
          DSW9.1   : ON
          DSW9.2   : OFF
          DSW9.3   : ON
          DSW9.4   : OFF

5. Hardware Connections:
    (1) Connect the CN26 port on evaluation board to the PC using USB Micro-B cable for debugging.
    (2) Connect the CN27 port on evaluation board to the PC using USB Mini-B cable for Termimal Emulator.
    (3) Connect the CN13 port on evaluation board to a 15V power source using a USB Type-C cable for board power supply.
    (4) Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.

Note:
1. Do not set the XSPI1 clock speed (XSPI_CLK1) to 12.5MHz. The project may not work properly.

2. Chip Erase is a time-consuming process, so it is commented out when the example project is provided.
   How to use Chip Erase:
   (1) Comment out the "Sector Erase" code block in hal_entry() function.
   (2) Uncomment the "Chip Erase" code block in hal_entry() function.
   (3) Re-build the project.
   (4) Download EP to RZ board and run the project. Then confirm Chip Erase operation via the Tera Term.

3. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none