**Navigation:** [🏠 ethernet_tas_notes](ethernet_tas_notes.md)

# RZT2N EVK – Board-Specific Guide

## Additional Hardware
* 1 x USB Type-A to micro USB cable (For CA55).

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

| # | RZT2N EVK Connector | Direction | PC NIC |
|---|---------------------|-----------|--------|
| 1 | J8.ETH0 | <span style="font-size:1.6em;">↔</span> | NIC0 |
| 2 | J8.ETH1 | <span style="font-size:1.6em;">↔</span> | NIC1 |

</div><br>
