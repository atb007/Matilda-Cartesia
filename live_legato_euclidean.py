#!/usr/bin/env python3
"""
Legato Euclidean phrase @ half speed → IAC → GarageBand.

Musical intent:
  - C major, slow 8th-note steps (half speed at 120 BPM DAW tempo)
  - One repeating 8-note phrase, looped
  - Euclidean gate (5-of-8) with gently shifting fills
  - Legato: gate 1.0 — each hit ties into the next
  - Subtle evolution: occasional neighbour-degree drift, soft velocity wander

Run (leave open):
  python3 live_legato_euclidean.py

GarageBand: arm Record on a software instrument track, Play, 120 BPM.
Ctrl+C to stop.
"""

import logging
import os
import random
import signal
import sys

from isobar import *
from isobar.io.midi.output import MidiOutputDevice

# --- Musical constants ---
KEY = Key("C", "major")
TEMPO = 120
STEP = 0.5  # 8th notes @ 120 BPM — half the rate of the 16th-note scripts
PHRASE_LEN = 8

# Warm ascending-descending line in scale degrees (C D E F G F E D)
PHRASE = [0, 2, 4, 5, 7, 5, 4, 2]
C_MAJOR_DEGREES = [0, 2, 4, 5, 7, 9, 11]

IAC_PORT = "IAC Driver Bus 1"


def subtly_evolve_degree(step: int, degree) -> int:
    """
    Mostly returns the phrase unchanged.
    At phrase boundaries, small chance to nudge to an adjacent scale degree.
    """
    if degree is None:
        return None

    if step % PHRASE_LEN != PHRASE_LEN - 1:
        return degree

    if random.random() > 0.22:
        return degree

    try:
        idx = C_MAJOR_DEGREES.index(degree)
    except ValueError:
        return degree

    nudge = random.choice([-1, 1])
    idx = max(0, min(len(C_MAJOR_DEGREES) - 1, idx + nudge))
    return C_MAJOR_DEGREES[idx]


def build_patterns():
    phrase = PLoop(PMapEnumerated(PSequence(PHRASE, 1), subtly_evolve_degree))

    # Same 5/8 feel, phase and density drift slowly — subtle, not disruptive
    rhythm = PLoop(
        PConcatenate(
            [
                PEuclidean(5, PHRASE_LEN),
                PEuclidean(5, PHRASE_LEN, phase=1),
                PEuclidean(6, PHRASE_LEN),
                PEuclidean(4, PHRASE_LEN),
                PEuclidean(5, PHRASE_LEN, phase=2),
            ]
        )
    )

    velocity = PLoop(PSequence([76, 70, 74, 68], 1) + PBrown(0, 1, -6, 6))

    return phrase, rhythm, velocity


def open_midi_out():
    if os.environ.get("USE_VIRTUAL") == "1":
        return MidiOutputDevice(device_name="Isobar Live Arp", virtual=True)
    return MidiOutputDevice(device_name=IAC_PORT)


def main():
    logging.basicConfig(level=logging.INFO, format="[%(asctime)s] %(message)s")

    midi_out = open_midi_out()
    print("Legato Euclidean phrase — C major @ half speed (120 BPM DAW)")
    print("MIDI → IAC Driver Bus 1 (or set USE_VIRTUAL=1)")
    print("GarageBand: Record arm + Play @ 120 BPM. Ctrl+C to stop.\n")

    phrase, rhythm, velocity = build_patterns()

    timeline = Timeline(TEMPO, output_device=midi_out)
    timeline.schedule(
        {
            "degree": phrase,
            "key": KEY,
            "octave": 4,
            "duration": STEP,
            "gate": 1.0,
            "amplitude": velocity,
            "active": rhythm,
        }
    )

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
