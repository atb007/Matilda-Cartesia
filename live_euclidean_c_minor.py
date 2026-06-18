#!/usr/bin/env python3
"""
Evolving Euclidean melody in C minor → IAC → GarageBand.

Musical intent:
  - C natural minor
  - Euclidean rhythm (shifting fills over 16-step cycles)
  - Melody slowly mutates toward the end of each phrase
  - Tempo ebbs and flows: 16th-note steps, then half-time 8th-note steps
  - On Euclidean rests, each step is randomly muted OR re-triggers the last pitch

Run (leave open):
  python3 live_euclidean_c_minor.py

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
KEY = Key("C", "minor")
TEMPO = 120
PHRASE_STEPS = 32
STEP_NORMAL = 0.25  # 16th notes @ 120 BPM
STEP_HALF = 0.5  # half-time feel (8th notes @ 120 BPM)

C_MINOR_DEGREES = [0, 2, 3, 5, 7, 8, 10]
HOLD_ON_REST_PROB = 0.58  # rest slot → re-trigger previous pitch vs mute

IAC_PORT = "IAC Driver Bus 1"


class PHoldOrMuteOnRest(Pattern):
    """
    Walk a pitch stream against a Euclidean gate.
    Hits advance the melody; rests randomly mute or hold the previous degree.
    """

    def __init__(self, degrees: Pattern, rhythm: Pattern, hold_prob: float = 0.5):
        self.degrees = degrees
        self.rhythm = rhythm
        self.hold_prob = hold_prob
        self.last_degree = C_MINOR_DEGREES[0]
        self.last_was_hold = False

    def __repr__(self):
        return f"PHoldOrMuteOnRest({self.degrees!r}, {self.rhythm!r})"

    def __next__(self):
        degree = Pattern.value(self.degrees)
        hit = Pattern.value(self.rhythm)

        if hit:
            if degree is not None:
                self.last_degree = degree
            self.last_was_hold = False
            return self.last_degree

        # Euclidean rest: mute or sustain previous pitch.
        if random.random() < self.hold_prob:
            self.last_was_hold = True
            return self.last_degree

        self.last_was_hold = False
        return None


def maybe_mutate_pitch(step: int, degree) -> int:
    """Toward the end of each phrase, more random scale degrees."""
    if degree is None:
        return None

    pos = step % PHRASE_STEPS
    prob = 0.06 + (pos / max(PHRASE_STEPS - 1, 1)) * 0.78

    if random.random() < prob:
        return random.choice(C_MINOR_DEGREES)
    return degree


def build_patterns():
    # Contour through C minor (C Eb F G Ab …)
    base_degrees = PPingPong(PSequence([0, 2, 3, 5, 7, 8, 5, 3], 1))
    degrees = PLoop(PMapEnumerated(base_degrees, maybe_mutate_pitch))

    # Shifting Euclidean meters — loop forever
    rhythm = PLoop(
        PConcatenate(
            [
                PEuclidean(5, 16),
                PEuclidean(7, 16),
                PEuclidean(4, 16),
                PEuclidean(9, 16),
                PEuclidean(6, 16),
            ]
        )
    )

    melody = PHoldOrMuteOnRest(degrees, rhythm, hold_prob=HOLD_ON_REST_PROB)

    # Ebb: full-speed 16ths, then flow: half-time 8ths
    duration = PLoop(
        PConcatenate(
            [
                PSequence([STEP_NORMAL] * 16, 1),
                PSequence([STEP_HALF] * 16, 1),
            ]
        )
    )

    velocity = PLoop(PSequence([88, 74, 82, 70], 1) + PBrown(0, 1, -10, 10))

    return melody, duration, velocity


def open_midi_out():
    if os.environ.get("USE_VIRTUAL") == "1":
        return MidiOutputDevice(device_name="Isobar Live Arp", virtual=True)
    return MidiOutputDevice(device_name=IAC_PORT)


def main():
    logging.basicConfig(level=logging.INFO, format="[%(asctime)s] %(message)s")

    midi_out = open_midi_out()
    print("Live Euclidean melody — C minor (ebb / flow @ 120 BPM)")
    print("MIDI → IAC Driver Bus 1 (or set USE_VIRTUAL=1)")
    print("GarageBand: Record arm + Play @ 120 BPM. Ctrl+C to stop.\n")

    melody, duration, velocity = build_patterns()

    timeline = Timeline(TEMPO, output_device=midi_out)
    timeline.schedule(
        {
            "degree": melody,
            "key": KEY,
            "octave": 4,
            "duration": duration,
            "gate": 0.92,
            "amplitude": velocity,
        }
    )

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
