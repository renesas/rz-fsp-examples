#!/usr/bin/env python3
"""
pad512.py - pad an SSBL .bin file to a multiple of 512 bytes (LOADER_UNIT_SIZE).

The RZ/T2N Boot ROM reads the SSBL image in 512-byte units, so the .bin file
must be a multiple of 512 bytes. The GCC output is already aligned this way,
but IAR ielftool does not guarantee it.

Run this script after the build and BEFORE step (1), parameter_generator.py loader.
If the file is already a multiple of 512 bytes, the script makes no changes.

Manual use:
    python pad512.py rzt2n_evk_ca55_ssbl.bin

EWARM post-build use (Options > Build Actions > Post-build command line;
configure it for ALL CONFIGURATIONS):
    python "$PROJ_DIR$\\script\\pad512.py" "$TARGET_BPATH$.bin"
"""

import os
import sys

UNIT = 512
FILL = 0x00          # Change to 0xFF if 0xFF padding is required.


def main(argv):
    if len(argv) != 2:
        print(__doc__)
        return 2

    path = argv[1]
    if not os.path.isfile(path):
        print("pad512: file not found: %s" % path)
        return 1

    size = os.path.getsize(path)
    rem = size % UNIT

    if rem == 0:
        print("pad512: %s = %d bytes (0x%X) - already a multiple of %d; no changes made."
              % (os.path.basename(path), size, size, UNIT))
        return 0

    pad = UNIT - rem
    with open(path, "ab") as f:
        f.write(bytes([FILL]) * pad)

    new = os.path.getsize(path)
    assert new % UNIT == 0, "pad512: padded size is not aligned; check the result"
    print("pad512: %s  %d -> %d bytes (0x%X), added %d bytes of 0x%02X."
          % (os.path.basename(path), size, new, new, pad, FILL))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
