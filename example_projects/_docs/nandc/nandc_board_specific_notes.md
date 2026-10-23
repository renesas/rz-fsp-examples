**Navigation:** [🏠 nandc_notes](nandc_notes.md)

# RZT2N EVK – Board-Specific Guide
## Additional Hardware

- 1 × RZT2N EVK board.
- 1 × USB Type-C cable.
- 2 × USB Type-A to Micro-B cables.
- 1 x Host PC.

## Hardware Configuration

### Boot Mode

xSPI1 boot mode (x1 boot serial flash)

### DIP Switch Settings

#### Operating Mode

| Switch | Setting |
|---------|---------|
| SW8.1 | ON |
| SW8.2 | OFF |
| SW8.3 | ON |
| SW8.4 | OFF |

#### 2nd Boot CPU :

| Switch | Setting | Boot CPU |
|---------|---------| ---------|
| SW10.10 | ON      | Cortex-R52 CPU0 |
| SW10.10 | OFF     | Cortex-A55 Core0|



#### NAND Configuration

| Switch  | Setting |
|---------|---------|
| SW7.7   | OFF     |
| SW7.8   | ON      |
| SW9.3   | ON      |

## Hardware Connections

### Debug and Power Connections

1. Connect the J32 connector on evaluation board to the PC using USB Micro-B cable for debugging.
2. Connect the J33 connector on evaluation board to the PC using USB Micro-B cable for serial terminal.
3. Connect the J7 connector on evaluation board to a 15V3A power source using a USB Type-C cable for board power supply.
4. Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
5. When want to turn off board, slide SW1 "POWER_SW" from ON to OFF side to turn off the power supply for evaluation board, then unplug USB Type-C cable out of J7 connector.

**Navigation:** [🏠 nandc_notes](nandc_notes.md) | [Supported Boards →](nandc_notes.md#supported-boards)

<br> <br>

