**Navigation:** [🏠 ddr_notes](ddr_notes.md)

# RZT2N EVK – Board-Specific Guide

## Additional Hardware
* 1 x USB Type-A to micro USB cable (For CA55, via the J33 UART-to-USB bridge).

## Hardware Configurations

### (1) Boot Mode Selection
Set the DIP switches for **xSPI1 boot mode**:

| Board     | Switch  | Position       |
|-----------|---------|----------------|
| RZT2N EVK | SW8-1   | ON             |
|           | SW8-2   | OFF            |
|           | SW8-3   | ON             |
|           | SW10-10 | ON (for CR52)  |
|           |         | OFF (for CA55) |

### (2) DDR / clock prerequisites (set in the FSP Configurator, not on the board)
* **BSP → onDDR = Enabled** — trains the LPDDR4 (`bsp_ddr_init()` / `BSP_CFG_DDR_INIT_ENABLE`).
* **Clocks → PLL2 = Released** — required by `bsp_ddr.h`; the build `#error`s otherwise.
* **Clocks → DDRCLK divider = /2** — DDRCLK 800 MHz.

## Specific Connections

<div style="margin-left:2em; border:1px solid #ccc; border-radius:4px;">

| # | RZT2N EVK Connector | Direction | PC |
|---|--------------------|-----------|----|
| 1 | POWER (USB Type-C) | ← | USB power source |
| 2 | J32 (USB Micro-B, on-board J-Link) | ↔ | PC USB port (debug + console, CR52 build) |
| 3 | J33 (USB Micro-B, UART-to-USB bridge) | ↔ | PC USB port (console, CA55 build) |

</div><br>
