#!/usr/bin/env python3

"""
Reads the reset count from the Arduino EEPROM.
"""

import argparse
import subprocess
import sys


def run():
    """
    Program entry point
    """

    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--programmer', default='avrispmkii',
        help="Type of programmer connected to the Arduino")
    args = parser.parse_args()

    cmd = ['avrdude', '-qq', '-p', 'atmega328p', '-c', args.programmer, '-P',
        'usb', '-U', 'eeprom:r:-:r']

    # Run the command silently first
    proc = subprocess.run(cmd, stdin=subprocess.DEVNULL, stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL)

    if proc.returncode != 0:
        # Make sure the programmer is attached
        print(f"Connect the {args.programmer!r} programmer to the computer "
            "and to the main microcontroller ISCP port.")
        input("Press Enter to continue. ")

        # This time show stderr to the user
        proc = subprocess.run(cmd, stdin=subprocess.DEVNULL,
            stdout=subprocess.PIPE)
        if proc.returncode != 0:
            sys.exit(1)

    output = proc.stdout
    if len(output) < 1024:
        sys.exit(f"Unexpected EEPROM size ({len(output)})")

    for i in range(0, 1024, 4):
        value = int.from_bytes(output[i:i + 4], 'little')
        if value == 0xFFFFFFFF:
            continue

        if value > 100000:
            sys.exit(f"Found probably uninitialized value {value:#08x}")

        print(f"{value} resets")
        return

    sys.exit("No reset count found (EEPROM was probably erased)")

if __name__ == '__main__':
    run()
