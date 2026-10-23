**Navigation:** [🏠 ethernet_frer_notes](ethernet_frer_notes.md)

# RZT2N EVK – Board-Specific Guide

## Additional Hardware
* 4 x USB Type-A to micro USB cables (two per board: one for J-Link debug via J32, one for the
  UART-to-USB console bridge via J33).

## Hardware Configurations

### (1) Boot Mode Selection

Set DIP switches to select **xSPI1 boot mode** and **peripheral pins** — identical on both boards:

| Board        | Switch   | Position      |
|--------------|----------|---------------|
| RZT2N EVK    | SW8-1    | ON            |
|              | SW8-2    | OFF           |
|              | SW8-3    | ON            |
|              | SW10-10  | ON (for CR52) |
|              |          | OFF (for CA55) |

## Specific Connections

This project pair is a **two-board** setup: one board runs the Sender project, the other runs
the Receiver project. There is no PC-side NIC connection at all — both external ports on both
boards connect directly to each other.

<div style="margin-left:2em; border:1px solid #ccc; border-radius:4px;">

| # | Sender board connector | Direction | Receiver board connector |
|---|---|---|---|
| 1 | J8A (ETH0) | <span style="font-size:1.6em;">↔</span> | J8A (ETH0) |
| 2 | J8B (ETH1) | <span style="font-size:1.6em;">↔</span> | J8B (ETH1) |

</div><br>

### Debug and Console Connections

1. Connect the **J32** connector on each evaluation board to the PC using a USB Micro-B cable
   for debugging (J-Link).
2. Connect the **J33** connector on each evaluation board to the PC using a USB Micro-B cable
   for the serial console — **not** the J32/J-Link USB, and **not** SEGGER RTT (see
   [Software Requirements](ethernet_frer_notes.md#software-requirements) for why this
   project uses UART/TeraTerm).
3. Connect the POWER (15V/3A) port on each evaluation board to a power source using a USB Type-C
   cable, then slide each board's SW1 "POWER_SW" from OFF to ON.

Each board's J33 UART-to-USB bridge enumerates on the host PC as a **pair** of COM ports (e.g.
COM5/COM6) — see [Application Execution](ethernet_frer_notes.md#application-execution)
for which one of the pair is the actual console port.
