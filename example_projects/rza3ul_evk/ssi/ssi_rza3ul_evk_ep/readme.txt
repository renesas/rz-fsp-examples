/***********************************************************************************************************************
* File Name    : readme.txt
* Description  : Contains general information about Example Project and detailed instructions.
***********************************************************************************************************************/

1. Project Overview:
    The example project demonstrates the typical use of the SSI HAL module APIs.
    The project transfers sample audio data from source buffer to destination buffer by connecting Tx and Rx pins
    and compares the transferred data of two buffers.
    The result of comparison will be displayed on Terminal Emulator.

    This project supports the DMAC feature. In cases where this feature is not used in the application,
    please disable the DMAC feature in FSP Configuration by the following steps.
        - Set DMAC Support to "Disabled" on the I2S (r_ssi) driver.
        - Remove the DMAC sub stacks that linked to the I2S (r_ssi) driver.

    Steps to enable again the DMAC feature in FSP Configuration after disabled it:
        - Set DMAC Support to "Enabled" on the I2S (r_ssi) driver.
        - Add Transfer (r_dmac) driver in the sub stacks that linked to the I2S (r_ssi) driver.
        - In the g_transfer0 SSIF_DMA_TX0, set DMA Activation Request Source Select to "Requested by a transfer
          destination module".
        - In the g_transfer1 SSIF_DMA_RX0, set DMA Activation Request Source Select to "Requested by a transfer
          source module".

2. Software Requirements:
    Renesas Flexible Software Package (FSP): Version 4.2.0
    e2 studio: Version 2026-07
    GCC ARM A-Profile (AArch64 bare-metal): Version 13.2 rel1
    Terminal Console Application: Tera Term or a similar application

3. Hardware Requirements:
    1 x RZ board.
    1 x USB Type-C cable.
    1 x USB Micro-B cable.
    1 x Jumper wire.

    RZ supported boards:
        RZ/A3UL Evaluation Board Kit QSPI Edition (Exec with DDR SDRAM).
        RZ/A3UL Evaluation Board Kit QSPI Edition (eXecute-In-Place).
        RZ/A3UL Evaluation Board Kit OCTAL Edition (eXecute-In-Place).

4. Hardware Configurations:
    (1) Boot Mode:
        Boot mode 3 (1.8-V Single, Quad, or Octal serial flash memory).

    (2) Set for DIP switches and jumpers as follow:
        +===============+====================================+
        | Board setting | RZ/A3UL Evaluation Board Kit       |
        +===============+====================================+
        | Module board  | SW1-1   : OFF                      |
        |               | SW1-2   : Don't care               |
        |               | SW1-3   : OFF                      |
        +---------------+------------------------------------+
        | Carrier board | SW1     : Don't care               |
        |               | SW2     : Don't care               |
        |               | SW3     : Don't care               |
        |               | SW4     : Don't care               |
        |               | SW5     : 1-2                      |
        |               | SW6     : 1-2                      |
        |               | SW7     : 1-2                      |
        |               | SW8     : 1-2                      |
        |               |                                    |
        |               | SW11-1  : OFF                      |
        |               | SW11-2  : OFF                      |
        |               | SW11-3  : OFF                      |
        |               | SW11-4  : ON                       |
        |               |                                    |
        |               | CN4     : Jumper connects 1-3      |
        |               |           Jumper connects 2-4      |
        +---------------+------------------------------------+

5. Hardware Connections:
    (1) Connect the CN6 port on the SMARC Carrier Board to a 5V power source using a USB Type-C cable
        for board power supply.
    (2) Connect the J-Link Base and the connector on the SMARC Module Board with a JTAG cable for debugging.
        When connecting the conversion adapter "J-Link 9-pin Cortex-M Adapter" made by SEGGER will be required.
    (3) Connect the CN14 port on the SMARC Carrier Board to the PC using a USB Micro-B cable for serial terminal
        communication (Tera Term).
    (4) Connect SSI0_TXD P2_2 (CN7:7) to SSI0_RXD P2_3 (CN7:8) using a jumper wire.

Note:
1. Set the configuration of Terminal Emulator (Tera Term) as follows:
   Port: is COM port provided by the RZ board.
   Speed: 115200 bps
   Data: 8 bit  
   Parity: none
   Stop bits: 1 bit
   Flow control: none