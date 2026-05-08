**Navigation:** [🏠 openh264_notes](openh264_notes.md)

# EK‑RZA3M – Board-Specific Guide

## Additional Hardware
* 1 x MIPI Graphics Expansion Board (e.g., MIPI Graphics Expansion Board: https://www.renesas.com/en/document/mat/mipi-graphics-expansion-board-2-v1-users-manual)

## Hardware Configurations

### (1) DIP Switch Settings
Set DIP switches to select **boot mode 4** and **peripheral pins**:

| Board        | Switch | Position | Note    |
|--------------|--------|----------|---------|
| Module board | SW4-1  | OFF      | Default |
|              | SW4-2  | OFF      | Default |
|              | SW4-3  | OFF      | Default |
|              | SW4-4  | OFF      | Default |
|              | SW4-5  | OFF      |         |
|              | SW5-1  | OFF      |         |
|              | SW5-2  | OFF      |         |
|              | SW5-3  | OFF      | Default |
|              | SW5-4  | OFF      |         |
|              | SW5-5  | OFF      | Default |

### (2) Jumper Configuration

| Jumper | Pins | Description                          |
|--------|------|--------------------------------------|
| J9     | 2–3  | Use on-board debug functionality     |


## Specific Connections

<div style="margin-left:2em; border:1px solid #ccc; border-radius:4px;">

| # |       Board Header       |                                         | MIPI Graphics Expansion Board Header      |
|---|--------------------------|-----------------------------------------|-------------------------------------------|
| 1 |           J32            | <span style="font-size:1.6em;">↔</span> |                     J1                    |

</div>

## Special Notes
None.

**Navigation:** [🏠 openh264_notes](openh264_notes.md) | [Supported Boards →](openh264_notes.md#supported-boards)

<br> <br>
