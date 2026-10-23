# RZT2N EVK Example Projects

This directory contains example projects for the **RZT2N EVK** (R9A07G076M48GBG, Cortex-A55).

## Available Example Projects

| Project | Core | Description |
|---------|------|-------------|
| [ethernet_frer_rzt2n_evk_ca55_0_receiver_ep](../ethernet_frer_rzt2n_evk_ca55_0_receiver_ep/) | CA55_0 | ESWM FRER (IEEE 802.1CB Frame Replication and Elimination for Reliability) — RECEIVER node: eliminates duplicate copies of the companion Sender's replicated frame arriving on ETH0/ETH1 via hardware sequence recovery. Run alongside the companion `ethernet_frer_rzt2n_evk_ca55_0_sender_ep` project on a second board. |

## Documents
| Document | Description | 🔗 Link |
|----------|-------------|--------|
| Ethernet FRER Notes | Overview and configuration | [View on GitHub](https://github.com/renesas/rz-fsp-examples/tree/main/example_projects/_docs/layer3_ethernet_switch/ethernet_frer/ethernet_frer_notes.md) |
| Board-Specific Guide | Board-specific details | [View on GitHub](https://github.com/renesas/rz-fsp-examples/tree/main/example_projects/_docs/layer3_ethernet_switch/ethernet_frer/ethernet_frer_board_specific_notes.md) |
