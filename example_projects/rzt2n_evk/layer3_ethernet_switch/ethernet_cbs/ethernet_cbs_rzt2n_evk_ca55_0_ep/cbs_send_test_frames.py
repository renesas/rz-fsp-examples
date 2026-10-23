"""
cbs_send_test_frames.py
------------------------
Sends raw Ethernet test frames (no IP) to the 2 destination MACs that
ethernet_cbs_rzt2n_evk_cr52_0_ep's CBS bandwidth table steers into queues Q0/Q1 on ETH0
(Q0 capped at 10% of link bandwidth, Q1 at 30% -- see cbs_config.c). Classification on the
board is by destination MAC only (MAC table entry), so EtherType/source MAC/payload content
don't matter for correctness -- only the destination MAC does.

The board picks its mode at runtime from a console menu. This script only does
anything useful while the board is running a port-to-port menu option: [1] (CBS disabled,
both queues' MAC entries active) or [2]/[3] (CBS enabled, ONE queue's entry active, the other
row excluded on the board -- pair with --queue 0/1 below so the offered traffic matches what's
actually classified). Options [4]-[6] ("CPU-generated") do NOT accept external traffic --
frames sent by this script while the board is in [4]-[6] will simply not appear on ETH0.

Port-to-port CBS shaping is confirmed working on both queues (Q0=10%/100 Mbps and
Q1=30%/300 Mbps), but ONLY when the offered load is driven well above the cap. This
Windows/Npcap script tops out around 200-270 Mbps, which is enough to see Q0's 100 Mbps clamp
but NOT enough margin to conclusively prove Q1's 300 Mbps cap -- use cbs_send_test_frames_linux.py
from a Raspberry Pi (raw AF_PACKET socket, >900 Mbps demonstrated) for the conclusive Q1 read.
See the CBS EP run guide for the full verification procedure.

CBS T2N's link is RGMII @ 1000 Mbps (Gigabit) -- see the CBS EP run guide -- so caps are a
much larger fraction of a Gbps than the low tens of Mbps a 100 Mbps link (e.g. MII) would give.
Reaching several hundred Mbps of sustained offered load from a Windows/Npcap/Scapy script needs two things
Python's naive per-packet sendp() loop doesn't have on its own:
  1. Pre-built RAW BYTES per frame (built once, sent many times) instead of Scapy Ether()/Raw()
     layer objects -- a plain Packet object still pays Scapy's per-packet layer-serialization
     cost on every send even when the same object is reused; raw bytes skip that entirely.
  2. A much larger batch per sendp() call (10000 vs. an earlier 500) -- fewer Python-level
     function-call/loop boundaries per packet actually sent.

Requirements:
    pip install scapy
    Npcap must already be installed (same driver Wireshark uses).
    Run this script's terminal AS ADMINISTRATOR (raw L2 send needs elevated rights on Windows).

Usage:
    1. First run with --list to find the exact interface name for the NIC
       physically connected to the board's ETH1 (J8B) port:
           python cbs_send_test_frames.py --list

    2. Quick connectivity check -- one frame per MAC, sent once:
           python cbs_send_test_frames.py --iface "Ethernet 3" --mode once

    3. Bandwidth demo -- fixed-duration flood (default 10 s), no pacing, as fast as
       Python/Npcap can send; prints the actually-achieved offered-load throughput per queue
       at the end, compared against that queue's CBS cap (100 Mbps / 300 Mbps on this board's
       Gigabit link) with an OK/WARNING verdict -- OK means enough offered load was generated
       to actually exercise the cap; WARNING means the script itself is the bottleneck, not
       CBS, and the run doesn't prove anything about the cap either way.
       Run this with the board in menu option [1] (CBS disabled) as the unshaped baseline, then
       use step 4 below for the per-queue CBS-enabled runs, and compare achieved throughput per
       MAC in Wireshark (Statistics -> I/O Graph, Y Axis = Bits, one graph line per dst MAC).
       Flooding both queues at once (--queue both) splits the sender's throughput between the
       two MACs, so each MAC's offered load is much lower than when flooding one queue alone --
       prefer --queue 0 / --queue 1 for anything you intend to read a result from:
           python cbs_send_test_frames.py --iface "Ethernet 3" --mode flood

    4. Isolated single-queue verification -- pair each of these with the matching
       board-side menu choice ([2] for Q0, [3] for Q1) so only one queue's MAC table entry
       exists on the board while only that queue's traffic is offered from the PC:
           python cbs_send_test_frames.py --iface "Ethernet 3" --mode flood --queue 0   # with board on [2]
           python cbs_send_test_frames.py --iface "Ethernet 3" --mode flood --queue 1   # with board on [3]

    5. (Optional) Change the flood duration (default 10 s):
           python cbs_send_test_frames.py --iface "Ethernet 3" --mode flood --duration 20
"""

import argparse
import time

from scapy.all import Ether, sendp, get_if_list, get_if_hwaddr

# get_windows_if_list moved out of scapy.all in newer Scapy versions -- import from its
# actual home and fall back to the cross-platform interface list if unavailable.
try:
    from scapy.arch.windows import get_windows_if_list
    HAS_WINDOWS_IF_LIST = True
except ImportError:
    HAS_WINDOWS_IF_LIST = False

# CBS T2N's link is Gigabit (RGMII @ 1000 Mbps) -- see the CBS EP run guide / configuration.xml
# (module.driver.rzt.ether_selector.speed.bps1000m). Unlike a 100 Mbps board, where a 10%/30%
# split would only be 10/30 Mbps, here it's 100/300 Mbps.
LINK_SPEED_MBPS = 1000

# Destination MACs from g_cbs_rows_uc1/uc2 (cbs_config.c) -> gated queue on ETH0, with each
# queue's CBS bandwidth percentage (band_width_pct in cbs_config.c) and the resulting absolute
# cap on this board's Gigabit link.
DST_MACS = [
    ("44:44:44:00:00:0a", "Q0", 10),   # CBS cap 10% of link speed = 100 Mbps
    ("44:44:44:00:00:0b", "Q1", 30),   # CBS cap 30% of link speed = 300 Mbps
]

SRC_MAC = "70:b3:d5:34:00:01"   # arbitrary source MAC placed in generated frames
ETHERTYPE_CBS_TEST = 0x88B5     # arbitrary -- MAC table classification doesn't look at EtherType

# "once" mode: minimal frame (60 B total, the Ethernet minimum).
ONCE_PAYLOAD_SIZE = 46
# "flood" mode: near-MTU frame (1514 B total) -- maximizes offered load per packet so the
# bandwidth cap becomes visible without needing a huge packet rate.
FLOOD_PAYLOAD_SIZE = 1500

# A much larger batch than an earlier version of this script used (500), to cut down on
# Python-level sendp() call/loop overhead relative to the number of packets actually sent.
FLOOD_BATCH_SIZE = 10000
FLOOD_DEFAULT_DURATION_S = 10.0


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
    for mac, queue, pct in DST_MACS:
        frame = Ether(dst=mac, type=ETHERTYPE_CBS_TEST) / (b"\x00" * ONCE_PAYLOAD_SIZE)
        print(f"Sending to {mac}  ({queue}, CBS cap {pct}% of link speed) ...")
        sendp(frame, iface=iface, verbose=True)
        time.sleep(delay_s)
    print("Done. Check Wireshark capture on the NIC connected to ETH0 (J8A).")


def _build_frame_bytes(dst_mac: str, payload_size: int) -> bytes:
    """Builds one raw Ethernet frame as plain bytes (no Scapy Packet object) -- built once,
    then reused for every send in the batch. Passing raw bytes (rather than a Scapy Packet,
    even a reused one) to sendp() skips Scapy's per-packet layer-serialization step on every
    send, which is what actually lets the flood below approach the throughput needed to
    exercise a 100/300 Mbps CBS cap; see the module docstring."""
    dst = bytes.fromhex(dst_mac.replace(":", ""))
    src = bytes.fromhex(SRC_MAC.replace(":", ""))
    ethertype = ETHERTYPE_CBS_TEST.to_bytes(2, "big")
    payload = b"\x11" * payload_size
    return dst + src + ethertype + payload


def send_flood(iface: str, queue: str, duration_s: float):
    """Bandwidth demo: blasts near-MTU frames as fast as possible for a fixed duration, no
    pacing/cycle-sync -- creates offered load intended to exceed the bandwidth cap on its own.
    Mirrors the board's own CPU-generated mode (cbs_send_frames() in cbs_ep.c) on the traffic
    pattern; send technique is raw pre-built bytes, large batch, fixed-duration measurement.

    queue selects which MAC(s) to send -- test each queue isolated first, then combined, to
    see both "queue gets its full share alone" and "queues don't starve each other". After the
    run, prints the actually-achieved offered-load throughput per queue against that queue's
    CBS cap, so you can tell whether the flood generated enough load to mean anything before
    ever opening Wireshark."""
    selected = DST_MACS if "both" == queue else [DST_MACS[int(queue)]]
    frames = [(_build_frame_bytes(mac, FLOOD_PAYLOAD_SIZE), mac, name, pct)
              for mac, name, pct in selected]
    frame_len = len(frames[0][0])   # all frames are the same length (FLOOD_PAYLOAD_SIZE-based)

    # Interleave evenly when sending more than one queue at once (A, B, A, B, ...) so the
    # two queues' offered load stays roughly balanced instead of one starving the other
    # purely from send ordering.
    per_slot = FLOOD_BATCH_SIZE // len(frames)
    batch = []
    for _ in range(per_slot):
        for frame_bytes, _mac, _name, _pct in frames:
            batch.append(frame_bytes)

    print(f"Flooding near-MTU frames ({', '.join(f'{name} ({mac})' for _fb, mac, name, _pct in frames)}) "
          f"for {duration_s:.0f} s. Batch size: {len(batch)} packets/call.\n")

    counts = {mac: 0 for _fb, mac, _name, _pct in frames}
    start = time.time()
    end = start + duration_s
    while time.time() < end:
        sendp(batch, iface=iface, verbose=False, realtime=False)
        for _fb, mac, _name, _pct in frames:
            counts[mac] += per_slot

    elapsed = time.time() - start
    print(f"\n[OFFERED-LOAD STATISTICS]  (measured at this PC's NIC, before any CBS shaping)")
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

    print("Now compare ACHIEVED throughput per MAC on the ETH0 (J8A) capture in Wireshark")
    print("(Statistics -> I/O Graph, Y Axis = Bits) between the board's CBS-disabled and")
    print("CBS-enabled menu options -- that capture, not this script's offered-load number,")
    print("is what actually proves whether the board is shaping traffic to the cap.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--list", action="store_true", help="List available NIC interface names and exit.")
    parser.add_argument("--iface", type=str, help="Interface name of the NIC connected to the board's ETH1 (J8B).")
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

    if args.list or not args.iface:
        list_interfaces()
        if not args.iface:
            print("\nPass --iface \"<name from the list above>\" to actually send test frames.")
    elif args.mode == "once":
        send_once(args.iface, args.delay)
    else:
        send_flood(args.iface, args.queue, args.duration)
