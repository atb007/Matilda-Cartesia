#!/usr/bin/env python3
"""
Escape hatch: keep raw isobar for anything Teleiso ops don't cover yet.

Usage:
  python3 teleiso_hybrid.py
"""

import os
import signal
import sys

from isobar import *
from isobar.io.midi.output import MidiOutputDevice

from teleiso import run_python, run_timeline


def build_custom():
    """Any isobar pattern — Teleiso doesn't need to know the syntax."""
    key = Key("C", "major")
    weird = PLoop(PMap(PSequence([0, 4, 7, 11], 1), lambda d: d + (1 if random.random() > 0.8 else 0)))
    return {
        "key": key,
        "degree": weird,
        "octave": 4,
        "duration": 0.125,
        "gate": 0.8,
        "amplitude": PLoop(PSequence([90, 75], 1)),
        "active": PLoop(PEuclidean(3, 8)),
    }


def main():
    patch = run_python(build_custom, tempo=120)
    port = "IAC Driver Bus 1"
    if os.environ.get("USE_VIRTUAL") == "1":
        midi = MidiOutputDevice(device_name="Isobar Live Arp", virtual=True)
    else:
        midi = MidiOutputDevice(device_name=port)

    print("Hybrid: Teleiso runner + raw isobar build_custom()")
    timeline = run_timeline(patch, output_device=midi)

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
