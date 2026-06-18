#!/usr/bin/env python3
"""List MIDI ports macOS sees. Run while live_arp_garageband.py is running in another window."""

import mido

print("=== MIDI OUTPUTS (isobar sends on these) ===")
for name in mido.get_output_names():
    print(" ", name)

print("\n=== MIDI INPUTS (DAW listens on these) ===")
for name in mido.get_input_names():
    print(" ", name)

print("\nIf 'IAC Driver Bus 1' is missing, re-open Audio MIDI Setup → IAC → Device is online → Apply.")
