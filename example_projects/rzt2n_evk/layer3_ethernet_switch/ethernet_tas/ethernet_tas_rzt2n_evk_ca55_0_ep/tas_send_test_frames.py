"""
tas_send_test_frames.py
------------------------
Sends raw Ethernet test frames (no IP) to the 4 destination MACs that
the TAS example's schedule steers into queues Q0..Q3 on ETH0
(each queue's gate opens for a different 5 ms slot in the 25 ms cycle -- see
tas_config.c). Classification on the board is by destination MAC only (MAC table
entry), so EtherType/source MAC/payload content don't matter for correctness --
only the destination MAC does.

The board now picks its use case at runtime from a console menu,
not at build time -- this script only does anything useful while the board is
currently running Option [1] "Port-to-port" or Option [3] "Combined" (both
accept external ETH1 traffic into the MAC table). Option [2] "CPU-generated"
does NOT accept external traffic -- frames sent by this script while the board
is in Option 2 will simply not appear on ETH0.

Requirements:
    pip install scapy
    Npcap must already be installed (same driver Wireshark uses).
    Run this script's terminal AS ADMINISTRATOR (raw L2 send needs elevated rights on Windows).

Usage:
    1. First run with --list to find the exact interface name for the NIC
       physically connected to the board's ETH1 (J8B) port:
           python tas_send_test_frames.py --list

    2. Quick connectivity check -- one frame per MAC, sent once:
           python tas_send_test_frames.py --iface "Ethernet 3" --mode once

    3. Continuous: one frame per queue every 25 ms cycle, forever, until Ctrl+C:
           python tas_send_test_frames.py --iface "Ethernet 3" --mode continuous
"""

import argparse
import ctypes
import sys
import time

from scapy.all import Ether, conf, sendp, get_if_list, get_if_hwaddr

# get_windows_if_list moved out of scapy.all in newer Scapy versions -- import from its
# actual home and fall back to the cross-platform interface list if unavailable.
try:
    from scapy.arch.windows import get_windows_if_list
    HAS_WINDOWS_IF_LIST = True
except ImportError:
    HAS_WINDOWS_IF_LIST = False

# Destination MACs from g_tas_rows_uc1/uc3 (tas_config.c) -> gated queue on ETH0
DST_MACS = [
    ("44:44:44:00:00:0a", "Q0  gate open [ 0.. 5] ms"),
    ("44:44:44:00:00:0b", "Q1  gate open [ 5..10] ms"),
    ("44:44:44:00:00:0c", "Q2  gate open [10..15] ms"),
    ("44:44:44:00:00:0d", "Q3  gate open [15..20] ms"),
]

ETHERTYPE_TAS_TEST = 0x88B5   # arbitrary -- MAC table classification doesn't look at EtherType
CYCLE_TIME_S = 0.025          # 25 ms -- must match TAS_CYCLE_TIME_NS in tas_config.h if that changes

# "once" mode: minimal frame (60 B total, the Ethernet minimum).
ONCE_PAYLOAD_SIZE = 46
# "continuous" mode: near-MTU frame (1514 B total) -- larger frames make the gate-open/close
# effect easier to see and exercise queueing more.
CONTINUOUS_PAYLOAD_SIZE = 1500


def list_interfaces():
    if HAS_WINDOWS_IF_LIST:
        for iface in get_windows_if_list():
            print(f"{iface['name']!r}  (description: {iface.get('description', '')})")
    else:
        # Fallback: plain cross-platform list (no friendly description, just the
        # interface identifier Scapy itself uses -- still valid for --iface).
        for name in get_if_list():
            try:
                mac = get_if_hwaddr(name)
            except Exception:
                mac = "N/A"
            print(f"{name!r}  (mac: {mac})")


def send_once(iface: str, delay_s: float):
    """Quick connectivity/steering check: one frame per MAC, sent once."""
    for mac, note in DST_MACS:
        frame = Ether(dst=mac, type=ETHERTYPE_TAS_TEST) / (b"\x00" * ONCE_PAYLOAD_SIZE)
        print(f"Sending to {mac}  ({note}) ...")
        sendp(frame, iface=iface, verbose=True)
        time.sleep(delay_s)
    print("Done. Check Wireshark capture on the NIC connected to ETH0 (J8A).")


def send_continuous(iface: str):
    """Send one frame per queue every TAS cycle, forever, until Ctrl+C."""
    frames = [
        Ether(dst=mac, type=ETHERTYPE_TAS_TEST) / (b"\x11" * CONTINUOUS_PAYLOAD_SIZE)
        for mac, _note in DST_MACS
    ]

    sock = conf.L2socket(iface=iface)
    if sys.platform == "win32":
        ctypes.windll.winmm.timeBeginPeriod(1)

    print(f"Sending 1 frame per queue (Q0..Q3) every {CYCLE_TIME_S * 1000:.0f} ms cycle.")
    print("Ctrl+C to stop. Watch Wireshark I/O Graph on the ETH0-connected NIC.\n")

    count = 0
    next_time = time.perf_counter()
    try:
        while True:
            for frame in frames:
                sock.send(frame)

            count += len(frames)
            if count % 100 == 0:
                print(f"Sent {count} packets")

            # Accumulate the boundary; coarse-sleep, then spin the last ~1.5 ms.
            next_time += CYCLE_TIME_S
            coarse = next_time - time.perf_counter() - 0.0015
            if coarse > 0:
                time.sleep(coarse)
            while time.perf_counter() < next_time:
                pass
    except KeyboardInterrupt:
        print(f"\nStopped. Total sent: {count} packets")
    finally:
        sock.close()
        if sys.platform == "win32":
            ctypes.windll.winmm.timeEndPeriod(1)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--list", action="store_true", help="List available NIC interface names and exit.")
    parser.add_argument("--iface", type=str, help="Interface name of the NIC connected to the board's ETH1 (J8B).")
    parser.add_argument("--mode", choices=["once", "continuous"], default="once",
                        help="'once': send each MAC one time (default, quick check). "
                             "'continuous': cycle-synced loop until Ctrl+C (gate-timing demo).")
    parser.add_argument("--delay", type=float, default=0.5,
                        help="[once mode only] delay in seconds between frames (default 0.5).")
    args = parser.parse_args()

    if args.list or not args.iface:
        list_interfaces()
        if not args.iface:
            print("\nPass --iface \"<name from the list above>\" to actually send test frames.")
    elif args.mode == "once":
        send_once(args.iface, args.delay)
    else:
        send_continuous(args.iface)
