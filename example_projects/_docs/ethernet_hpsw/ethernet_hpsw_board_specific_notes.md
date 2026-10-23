**Navigation:** [🏠 ethernet_hpsw_notes](ethernet_hpsw_notes.md)

# RZT2N EVK – Board-Specific Guide

#### Unsupported Board Variant

The following RZT2N board does not support HPSW:

- RTK9RZT2N0CW1010BJ

## Additional Hardware

- 2 × RZT2N EVK boards.
- 2 × USB Type-C cables.
- 4 × USB Type-A to Micro-B cables.
- 3 × Ethernet cables.
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

#### Connector Settings

| Connector  | Setting    |
|------------|------------|
| J18        | 1-2 short  |

#### Ethernet Configuration

| Switch | Setting |
|---------|---------|
| SW7.3 | ON |
| SW7.4 | ON |
| SW7.7 | ON |
| SW7.8 | ON |
| SW10.3 | ON |

## Hardware Connections
   
### Network Connections

#### PRP Configuration

| Connection | RZ/T2N_EVK Board A Port | RZ/T2N_EVK Board B Port |
|------------|--------------|--------------|
| HPSW LAN A | ETH3         | ETH3         |
| HPSW LAN B | ETH4         | ETH4         |
| GMAC       | ETH0         | ETH0         |

#### HSR Configuration

| Connection | RZ/T2N_EVK Board A Port | RZ/T2N_EVK Board B Port |
|------------|--------------|--------------|
| HSR LAN A  | ETH3         | ETH4         |
| HSR LAN B  | ETH4         | ETH3         |
| GMAC       | ETH0         | ETH0         |

### Debug and Power Connections

1. Connect ETH0, ETH3, and ETH4 ports on 2 Renesas RZ boards to each other using Ethernet cables.
2. Connect the J32 connector on each evaluation board to the PC using USB Micro-B cable for debugging.
3. Connect the J33 connector on each evaluation board to the PC using USB Micro-B cable for serial terminal.
4. Connect the J7 connector on each evaluation board to a 15V3A power source using a USB Type-C cable for board power supply.
5. Slide SW1 "POWER_SW" from OFF to ON side to turn on the power supply for evaluation board.
6. When want to turn off board, slide SW1 "POWER_SW" from ON to OFF side to turn off the power supply for evaluation board, then unplug USB Type-C cable out of J33 connector.
## Special Notes


**Navigation:** [🏠 ethernet_hpsw_notes](ethernet_hpsw_notes.md) | [Supported Boards →](ethernet_hpsw_notes.md#supported-boards)

<br> <br>

