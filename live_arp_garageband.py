#!/usr/bin/env python3
"""
Live arpeggio → MIDI → GarageBand (no hardware keyboard).

1. Run (keep Terminal open):
     python3 live_arp_garageband.py

2. GarageBand:
     - Settings → Audio/MIDI → enable "IAC Driver Bus 1"
     - Software instrument track → click Record (red) → press Play
     - Tempo 120 BPM

3. Ctrl+C in Terminal to stop.

Uses IAC Driver Bus 1 by default (enable in Audio MIDI Setup).
Set USE_VIRTUAL=1 to use a "Isobar Live Arp" port instead.
"""

import logging
import os
import signal
import sys

from isobar import *
from isobar.io.midi.output import MidiOutputDevice

IAC_PORT = "IAC Driver Bus 1"
VIRTUAL_PORT = "Isobar Live Arp"


def open_midi_out():
    if os.environ.get("USE_VIRTUAL") == "1":
        return MidiOutputDevice(device_name=VIRTUAL_PORT, virtual=True), VIRTUAL_PORT
    return MidiOutputDevice(device_name=IAC_PORT), IAC_PORT


def main():
    logging.basicConfig(level=logging.INFO, format="[%(asctime)s] %(message)s")

    midi_out, port_label = open_midi_out()
    print(f"MIDI output: {port_label}")
    print("GarageBand → Settings → Audio/MIDI → enable that port.")
    print("Arm track (Record), press Play. Ctrl+C here to stop.\n")

    key = Key("C", "minor")
    arpeggio = PSeries(0, 2, 4)
    arpeggio = PDegree(arpeggio, key) + 60
    arpeggio = PPingPong(arpeggio)
    arpeggio = PLoop(arpeggio)

    timeline = Timeline(120, output_device=midi_out)
    timeline.schedule(
        {
            "note": arpeggio,
            "duration": 0.25,
            "amplitude": PSequence([90, 70, 60, 70]),
        }
    )

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
