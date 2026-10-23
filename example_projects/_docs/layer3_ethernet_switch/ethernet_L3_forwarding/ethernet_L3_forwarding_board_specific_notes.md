**Navigation:** [🏠 ethernet_L3_forwarding_notes](ethernet_L3_forwarding_notes.md)

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

| # | RZT2N EVK Connector | Direction | PC NIC | Subnet |
|---|---------------------|-----------|--------|--------|
| 1 | J8.ETH0 (RMAC0) | <span style="font-size:1.6em;">↔</span> | NIC1 | 192.168.10.x |
| 2 | J8.ETH1 (RMAC1) | <span style="font-size:1.6em;">↔</span> | NIC2 | 192.168.20.x |

</div><br>

## NIC IP Configuration

Configure static IP addresses on the PC before running the EP. These settings are the same for all forwarding modes (MAC, VLAN, L3):

| NIC | Connected To | IP Address | Subnet Mask | Gateway |
|-----|-------------|------------|-------------|---------|
| NIC1 | J8.ETH0 | 192.168.10.100 | 255.255.255.0 | 192.168.10.1 |
| NIC2 | J8.ETH1 | 192.168.20.100 | 255.255.255.0 | 192.168.20.1 |

**On Windows:**
1. Open **Network Connections** (ncpa.cpl).
2. Right-click the NIC -> Properties -> Internet Protocol Version 4 (TCP/IPv4) -> Properties.
3. Select "Use the following IP address" and enter values from the table above.

## MAC Address Configuration

The EP uses the PC NIC MAC addresses as forwarding table lookup keys. Before building:

1. Open Command Prompt on the Windows PC and run `ipconfig /all`.
2. Find the Physical Address of NIC1 and NIC2.
3. Update the corresponding MAC address constants in `src/L3forwarding.c`.
   ```c
   static const uint8_t g_l3_forwarding_pc1_mac[6] = {0x00, 0x0E, 0xC6, 0xC8, 0x3C, 0x29}; /* NIC1 */
   static const uint8_t g_l3_forwarding_pc2_mac[6] = {0x00, 0x23, 0x56, 0x3C, 0x86, 0x51}; /* NIC2 */
   ```
4. Rebuild the project.

## Special Notes

* ETH0 board MAC address: `00:11:22:33:44:00` (set in FSP configuration `g_layer3_switch0_mac_address_port0`)
* ETH1 board MAC address: `00:11:22:33:44:01` (set in FSP configuration `g_layer3_switch0_mac_address_port1`)
* The CPU_INJECT port (index 2) is internal to the ESWM and has no physical connector on the board.
* For Wireshark capture on PC, start capturing on NIC1 or NIC2 before entering a forwarding mode on the console.

**Navigation:** [🏠 ethernet_L3_forwarding_notes](ethernet_L3_forwarding_notes.md) | [Supported Boards →](ethernet_L3_forwarding_notes.md#supported-boards)

<br> <br>
