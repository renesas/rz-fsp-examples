"""
cbs_send_test_frames_linux.py
------------------------------
Linux/Raspberry Pi version of cbs_send_test_frames.py -- sends raw Ethernet test frames (no IP)
to the 2 destination MACs that ethernet_CBS_rzt2n_evk_cr52_0_ep's CBS bandwidth table steers
into queues Q0/Q1 on the board's destination port (Q0 capped at 10% of link bandwidth, Q1 at
30% -- see cbs_config.c). Classification on the board is by destination MAC only (MAC table
entry), so EtherType/source MAC/payload content don't matter for correctness -- only the
destination MAC does.

WHY THIS EXISTS -- same test, different transport:
The Windows version (cbs_send_test_frames.py) tops out at roughly 200-270 Mbps solo, even with
raw pre-built bytes and large sendp() batches, because it still goes through Npcap's driver
stack. That ceiling has been the limiting factor for conclusively testing Q1's 300 Mbps cap on
the port-to-port path throughout this project. Linux's AF_PACKET raw socket interface sends
frames directly through the kernel's native packet path -- no pcap/Npcap layer in between --
and reference_RA's own original CBS PoC script (OneDrive_2026-06-24/RA8X2 Examples/TSN/CBS/
cbs_example.py) used exactly this approach before the project moved to a Windows-only script
(RA's own caps were only 10/30 Mbps on a 100 Mbps link, so Windows was fast enough there and
simplicity won out -- that trade-off doesn't hold for RZT2N's 100/300 Mbps caps on a Gigabit
link). This script ports that raw-socket technique forward, with the same offered-load
verification, --queue isolation, and CLI shape as the Windows script, so results are directly
comparable.

Requirements:
    Linux only (Raspberry Pi OS, Ubuntu, etc.) -- AF_PACKET is a Linux-specific socket family,
    this will not run on Windows or macOS.
    Run as root (sudo) -- AF_PACKET SOCK_RAW requires elevated privileges, same as the RA
    reference script.
    No external packages needed -- uses only the Python 3 standard library (unlike the Windows
    version, which needs Scapy).

Usage:
    1. Find the interface name connected to the board's ETH1 (J8B) port:
           ip link show
       (Raspberry Pi's built-in port is usually "eth0"; a USB NIC often shows as "eth1" or
       "enxAABBCCDDEEFF" -- confirm by checking which one shows carrier/link when the board's
       ETH1 cable is plugged in: `ip link show <iface>` -- look for "state UP".)

    2. Quick connectivity check -- one frame per MAC, sent once:
           sudo python3 cbs_send_test_frames_linux.py --iface eth0 --mode once

    3. Bandwidth demo -- fixed-duration flood (default 10 s), no pacing, as fast as the raw
       socket can send; prints the actually-achieved offered-load throughput per queue at the
       end, compared against that queue's CBS cap (100 Mbps / 300 Mbps on this board's Gigabit
       link) with an OK/WARNING verdict, same convention as the Windows script:
           sudo python3 cbs_send_test_frames_linux.py --iface eth0 --mode flood

    4. Isolated single-queue verification -- pair with the matching board-side menu choice
       ([2] for Q0, [3] for Q1) so only one queue's MAC table entry exists on the board while
       only that queue's traffic is offered from this machine:
           sudo python3 cbs_send_test_frames_linux.py --iface eth0 --mode flood --queue 0   # with board on [2]
           sudo python3 cbs_send_test_frames_linux.py --iface eth0 --mode flood --queue 1   # with board on [3]

    5. Change the flood duration (default 10 s):
           sudo python3 cbs_send_test_frames_linux.py --iface eth0 --mode flood --duration 20

If this still can't clear 300 Mbps solo: try running two instances at once in separate
terminals (as with the Windows script) -- but note the CAVEAT proven earlier in this project's
testing: two independent senders sharing one NIC can hit a sender-side ceiling of their own,
which looks identical to a real CBS cap unless you also capture the SAME combined offered load
with the board in a CBS-disabled mode and confirm the achieved throughput actually differs.
Always do that disabled-vs-enabled comparison before trusting a multi-process combined result.
"""

import argparse
import os
import socket
import sys
import time

ETH_P_ALL = 0x0003  # capture/send all ethertypes at L2

# CBS T2N's link is Gigabit (RGMII @ 1000 Mbps) -- see the CBS EP run guide / configuration.xml
# (module.driver.rzt.ether_selector.speed.bps1000m).
LINK_SPEED_MBPS = 1000

# Destination MACs from g_cbs_rows_uc1/uc2 (cbs_config.c) -> gated queue on the board's
# destination port, with each queue's CBS bandwidth percentage and the resulting absolute cap.
DST_MACS = [
    ("44:44:44:00:00:0a", "Q0", 10),   # CBS cap 10% of link speed = 100 Mbps
    ("44:44:44:00:00:0b", "Q1", 30),   # CBS cap 30% of link speed = 300 Mbps
]

SRC_MAC = "70:b3:d5:34:00:01"       # arbitrary source MAC placed in generated frames
ETHERTYPE_CBS_TEST = 0x88B5         # arbitrary -- MAC table classification doesn't look at it
                                     # (same value the Windows script uses, for consistency)

ONCE_PAYLOAD_SIZE = 46              # "once" mode: minimal frame (60 B total, Ethernet minimum)
FLOOD_FRAME_SIZE_MAX = 1514         # "flood" mode: near-MTU frame, matches the Windows script's
                                     # CBS_FRAME_SIZE_MAX convention for directly comparable results
FLOOD_DEFAULT_DURATION_S = 10.0


def mac_to_bytes(mac: str) -> bytes:
    return bytes.fromhex(mac.replace(":", ""))


def set_promisc(iface: str, enable: bool) -> None:
    """Best-effort promiscuous-mode toggle via the modern `ip` tool (falls back to ifconfig,
    matching the RA reference script, if `ip` isn't available). Not strictly required for
    *sending* raw frames, but matches the RA script's setup and avoids surprises if this
    machine's NIC driver behaves differently without it."""
    state = "on" if enable else "off"
    if os.system(f"ip link set {iface} promisc {state} >/dev/null 2>&1") != 0:
        os.system(f"ifconfig {iface} {'promisc' if enable else '-promisc'} >/dev/null 2>&1")


def open_raw_socket(iface: str) -> socket.socket:
    if os.geteuid() != 0:
        print("[ERROR] This script needs root privileges for AF_PACKET raw sockets.")
        print("        Re-run with: sudo python3 cbs_send_test_frames_linux.py ...")
        sys.exit(1)

    sock = socket.socket(socket.AF_PACKET, socket.SOCK_RAW, socket.htons(ETH_P_ALL))
    sock.bind((iface, 0))
    return sock


def build_frame_bytes(dst_mac: str, payload_size: int) -> bytes:
    """Builds one raw Ethernet frame as plain bytes -- built once, sent many times. Same
    approach as the Windows script's _build_frame_bytes(), kept for directly comparable
    results between the two transports."""
    dst = mac_to_bytes(dst_mac)
    src = mac_to_bytes(SRC_MAC)
    ethertype = ETHERTYPE_CBS_TEST.to_bytes(2, "big")
    payload = b"\x11" * payload_size
    return dst + src + ethertype + payload


def send_once(sock: socket.socket, delay_s: float) -> None:
    """Quick connectivity/steering check: one frame per MAC, sent once."""
    for mac, queue, pct in DST_MACS:
        frame = build_frame_bytes(mac, ONCE_PAYLOAD_SIZE)
        print(f"Sending to {mac}  ({queue}, CBS cap {pct}% of link speed) ...")
        sock.send(frame)
        time.sleep(delay_s)
    print("Done. Check Wireshark capture on the NIC connected to the board's destination port.")


def send_flood(sock: socket.socket, queue: str, duration_s: float) -> None:
    """Bandwidth demo: blasts near-MTU frames as fast as the raw socket will send, for a fixed
    duration, no pacing/cycle-sync -- mirrors the Windows script's send_flood() so results are
    directly comparable. queue selects which MAC(s) to send; 'both' interleaves them evenly."""
    selected = DST_MACS if queue == "both" else [DST_MACS[int(queue)]]
    frames = [(build_frame_bytes(mac, FLOOD_FRAME_SIZE_MAX - 14), mac, name, pct)
              for mac, name, pct in selected]
    frame_len = len(frames[0][0])

    print(f"Flooding near-MTU frames ({', '.join(f'{name} ({mac})' for _fb, mac, name, _pct in frames)}) "
          f"for {duration_s:.0f} s via AF_PACKET raw socket.\n")

    counts = {mac: 0 for _fb, mac, _name, _pct in frames}
    n = len(frames)
    start = time.time()
    end = start + duration_s
    i = 0
    while time.time() < end:
        fb, mac, _name, _pct = frames[i % n]
        sock.send(fb)  # default blocking socket -- send() blocks until the driver accepts the
                        # frame, which is exactly the throttling behavior we want here (no
                        # separate pacing needed, matches the RA reference script's approach)
        counts[mac] += 1
        i += 1

    elapsed = time.time() - start
    print(f"[OFFERED-LOAD STATISTICS]  (measured at this machine's NIC, before any CBS shaping)")
    print(f"  Duration:     {elapsed:.2f} s")
    print(f"  Frame size:   {frame_len} bytes\n")

    for _fb, mac, name, pct in frames:
        cap_mbps = LINK_SPEED_MBPS * pct / 100.0
        sent = counts[mac]
        mbps = (sent * frame_len * 8) / (elapsed * 1_000_000)
        verdict = "OK -- offered load exceeds the cap, run is meaningful" if mbps > cap_mbps else \
                  "WARNING -- offered load below the cap; this run can't prove the cap works"
        print(f"  {name} ({mac}):")
        print(f"    Packets sent:  {sent:,}")
        print(f"    Offered load:  {mbps:.1f} Mbps")
        print(f"    CBS cap:       {cap_mbps:.0f} Mbps  ({pct}% of {LINK_SPEED_MBPS} Mbps link)")
        print(f"    {verdict}\n")

    print("Now compare ACHIEVED throughput per MAC on the board's destination-port capture in")
    print("Wireshark (Statistics -> I/O Graph, Y Axis = Bits) between the board's CBS-disabled")
    print("and CBS-enabled menu options -- that capture, not this script's offered-load number,")
    print("is what actually proves whether the board is shaping traffic to the cap.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--iface", type=str, required=True,
                        help="Interface name connected to the board's ETH1 (J8B), e.g. eth0.")
    parser.add_argument("--mode", choices=["once", "flood"], default="once",
                        help="'once': send each MAC one time (default, quick check). "
                             "'flood': fixed-duration unpaced blast (bandwidth demo).")
    parser.add_argument("--delay", type=float, default=0.5,
                        help="[once mode only] delay in seconds between frames (default 0.5).")
    parser.add_argument("--queue", choices=["0", "1", "both"], default="both",
                        help="[flood mode only] which queue's MAC to flood: '0' (..0A, 100 Mbps "
                             "cap) only, '1' (..0B, 300 Mbps cap) only, or 'both' at once "
                             "(default) -- test each in isolation, then combined.")
    parser.add_argument("--duration", type=float, default=FLOOD_DEFAULT_DURATION_S,
                        help=f"[flood mode only] flood duration in seconds (default "
                             f"{FLOOD_DEFAULT_DURATION_S:.0f}).")
    args = parser.parse_args()

    if not sys.platform.startswith("linux"):
        print("[ERROR] AF_PACKET raw sockets are Linux-only. Use cbs_send_test_frames.py on Windows.")
        sys.exit(1)

    set_promisc(args.iface, True)
    s = open_raw_socket(args.iface)
    try:
        if args.mode == "once":
            send_once(s, args.delay)
        else:
            send_flood(s, args.queue, args.duration)
    finally:
        s.close()
        set_promisc(args.iface, False)
