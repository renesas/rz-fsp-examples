**Navigation:** [🏠 ethernet_cbs_notes](ethernet_cbs_notes.md)

# RZT2N EVK – Board-Specific Guide

## Additional Hardware
* 1 x USB Type-A to micro USB cable (For CA55, via the J33 UART-to-USB bridge).
* A traffic generator able to send several hundred Mbps of raw Ethernet frames into ETH1 -
  needed only for the port-to-port modes `[1]`-`[3]`. A Raspberry Pi with a USB3 Gigabit Ethernet
  adapter is the validated choice (see
  [Traffic Generator Requirements](ethernet_cbs_notes.md#traffic-generator-requirements)).

## Hardware Configurations

### (1) Boot Mode Selection
Set DIP switches to select **xSPI1 boot mode** and **peripheral pins**:

| Board        | Switch   | Position      |
|--------------|----------|---------------|
| RZT2N EVK    | SW8-1    | ON            |
|              | SW8-2    | OFF           |
|              | SW8-3    | ON            |
|              | SW10-10  | ON (for CR52) |
|              |          | OFF(for CA55) |

## Specific Connections

<div style="margin-left:2em; border:1px solid #ccc; border-radius:4px;">

| # | RZT2N EVK Connector | Direction | Peer | Used by |
|---|---------------------|-----------|------|---------|
| 1 | J8.ETH0 (Port 0, CBS-shaped egress) | <span style="font-size:1.6em;">↔</span> | Capture PC NIC (Wireshark) | all modes |
| 2 | J8.ETH1 (Port 1, external ingress) | <span style="font-size:1.6em;">↔</span> | Traffic generator NIC (Raspberry Pi USB3 adapter, or a PC NIC) | modes `[1]`-`[3]` only |

</div><br>

**Important:** cable the traffic generator **directly** to ETH1 - no switch or router in between.
The test frames use synthetic destination MACs (`44:44:44:00:00:0A` / `..0B`) that no real
switch ever "learns"; a managed switch's unknown-unicast blocking silently drops every one of
them, which looks exactly like a board fault (the generator reports full speed while Wireshark on
ETH0 sees nothing).

### Debug and Console Connections

1. Connect the **J32** connector to the PC using a USB Micro-B cable for debugging (J-Link).
2. Console output and menu input:
   * **Cortex-R52:** go through **SEGGER RTT** over the same J-Link connection (open SEGGER J-Link
     RTT Viewer, target `R9A07G076`, [RTT Control Block] **Address** = the SEGGER_RTT address in
     [Supported Boards](ethernet_cbs_notes.md#supported-boards)).
   * **Cortex-A55:** connect the **J33** connector (UART-to-USB bridge) to the PC using a USB
     Micro-B cable and open Tera Term on its COM port (see
     [Application Execution](ethernet_cbs_notes.md#application-execution)).
3. Connect the POWER (15V/3A) port to a power source using a USB Type-C cable, then slide SW1
   "POWER_SW" from OFF to ON.
