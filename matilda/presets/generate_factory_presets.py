#!/usr/bin/env python3
"""Generate Matilda v2 factory presets (trance / 3rds / 5ths / polyrhythm)."""

from __future__ import annotations

import json
from pathlib import Path
from typing import Any, Dict, List, Optional, Sequence, Tuple

Note = Tuple[int, int, bool, int]  # degree, octave_offset, gate, velocity


def cell(
    degree: int = 0,
    *,
    gate: bool = True,
    velocity: int = 90,
    octave_offset: int = 0,
    trigger_armed: bool = False,
    trigger_prob: float = 0.5,
    jitter_armed: bool = False,
    jitter_amount: float = 0.5,
) -> Dict[str, Any]:
    return {
        "degree": degree,
        "gate": gate,
        "velocity": velocity,
        "octave_offset": octave_offset,
        "trigger_armed": trigger_armed,
        "trigger_prob": trigger_prob,
        "jitter_armed": jitter_armed,
        "jitter_amount": jitter_amount,
    }


def grid_from_notes(notes: Sequence[Note], fill: int = 16) -> List[List[Dict[str, Any]]]:
    """Row-major 4×4 from a linear note list. Pads / truncates to 16 cells."""
    padded: List[Note] = list(notes[:fill])
    while len(padded) < 16:
        padded.append((0, 0, False, 70))
    rows: List[List[Dict[str, Any]]] = []
    for y in range(4):
        row: List[Dict[str, Any]] = []
        for x in range(4):
            d, o, g, v = padded[y * 4 + x]
            row.append(cell(d, gate=g, velocity=v, octave_offset=o))
        rows.append(row)
    return rows


def empty_layer() -> Dict[str, Any]:
    return {
        "active": False,
        "movement": "forward",
        "random_skip_prob": 0.0,
        "active_step_count": 16,
        "step_index": 0,
        "step_dir": 1,
    }


def make_layer(
    notes: Sequence[Note],
    *,
    active: bool = True,
    movement: str = "forward",
    active_step_count: Optional[int] = None,
    random_skip_prob: float = 0.0,
) -> Dict[str, Any]:
    n = active_step_count if active_step_count is not None else min(16, max(1, len(notes)))
    return {
        "active": active,
        "movement": movement,
        "random_skip_prob": random_skip_prob,
        "active_step_count": n,
        "step_index": 0,
        "step_dir": 1,
        "cells": grid_from_notes(notes, fill=16),
    }


def patch(
    title: str,
    *,
    root: str,
    mode: str,
    layers: List[Dict[str, Any]],
    master_division: float = 0.0625,
    polyphony: bool = False,
    min_octave: int = 2,
    max_octave: int = 6,
    play_mode: str = "note",
) -> Dict[str, Any]:
    while len(layers) < 4:
        layers.append(empty_layer())
    return {
        "title": title,
        "version": 2,
        "root": root,
        "mode": mode,
        "quantize": True,
        "min_octave": min_octave,
        "max_octave": max_octave,
        "master_division": master_division,
        "play_mode": play_mode,
        "play_on_transport": False,
        "selected_layer": 0,
        "polyphony": polyphony,
        "poly_voices": 4 if polyphony else 1,
        "layers": layers[:4],
        "seed": None,
    }


def n(degree: int, oct_: int = 0, gate: bool = True, vel: int = 90) -> Note:
    return (degree, oct_, gate, vel)


# Scale-degree helpers (7-note modes): 0=1, 1=2, 2=3, 3=4, 4=5, 5=6, 6=7
R, _2, _3, _4, _5, _6, _7 = 0, 1, 2, 3, 4, 5, 6


def build_presets() -> Dict[str, Dict[str, Any]]:
    presets: Dict[str, Dict[str, Any]] = {}

    # Classic trance 6-note rotation: R-3-5-8-5-3 (BeatKey / common pluck arp)
    trance_cycle = [
        n(R, 0, True, 92),
        n(_3, 0, True, 78),
        n(_5, 0, True, 88),
        n(R, 1, True, 100),
        n(_5, 0, True, 82),
        n(_3, 0, True, 74),
    ]
    trance_16: List[Note] = []
    for i in range(16):
        trance_16.append(trance_cycle[i % 6])
    # Accent every downbeat slightly already via velocity; add a gated breath on last 16th of each half
    trance_16[7] = n(_3, 0, False, 70)
    trance_16[15] = n(R, 1, False, 70)
    presets["Trance Classic"] = patch(
        "Trance Classic — R·3·5·8 pluck arp",
        root="A",
        mode="minor",
        master_division=0.0625,
        min_octave=3,
        max_octave=6,
        layers=[make_layer(trance_16, movement="forward", active_step_count=16)],
    )

    # Uplifting variant with octave peek + ping-pong
    rise = [
        n(R, 0, True, 86),
        n(_3, 0, True, 80),
        n(_5, 0, True, 90),
        n(R, 1, True, 98),
        n(_3, 1, True, 92),
        n(_5, 1, True, 100),
        n(R, 2, True, 110),
        n(_5, 1, True, 88),
        n(_3, 1, True, 82),
        n(R, 1, True, 90),
        n(_5, 0, True, 84),
        n(_3, 0, True, 78),
        n(R, 0, True, 86),
        n(_5, 0, True, 80),
        n(_3, 0, True, 76),
        n(R, 0, True, 84),
    ]
    presets["Trance Rise"] = patch(
        "Trance Rise — ascending then cascade",
        root="A",
        mode="minor",
        master_division=0.0625,
        min_octave=3,
        max_octave=7,
        layers=[make_layer(rise, movement="ping_pong", active_step_count=16)],
    )

    # Stacked diatonic 3rds — “3rds heaven”
    thirds = [
        n(R, 0, True, 78),
        n(_3, 0, True, 84),
        n(_5, 0, True, 90),
        n(_7, 0, True, 96),
        n(_2, 1, True, 100),
        n(_4, 1, True, 104),
        n(_6, 1, True, 108),
        n(R, 2, True, 112),
        n(_6, 1, True, 100),
        n(_4, 1, True, 94),
        n(_2, 1, True, 88),
        n(_7, 0, True, 84),
        n(_5, 0, True, 80),
        n(_3, 0, True, 76),
        n(R, 0, True, 82),
        n(_5, 0, True, 78),
    ]
    # Harmony layer: same cascade starting a 3rd higher, 12-step loop against 16
    thirds_harm = [
        n(_3, 0, True, 70),
        n(_5, 0, True, 74),
        n(_7, 0, True, 78),
        n(_2, 1, True, 82),
        n(_4, 1, True, 86),
        n(_6, 1, True, 90),
        n(R, 2, True, 94),
        n(_3, 2, True, 88),
        n(R, 2, True, 84),
        n(_6, 1, True, 80),
        n(_4, 1, True, 76),
        n(_2, 1, True, 72),
    ]
    presets["3rds Heaven"] = patch(
        "3rds Heaven — stacked thirds lattice",
        root="F",
        mode="lydian",
        master_division=0.0625,
        polyphony=True,
        min_octave=3,
        max_octave=7,
        layers=[
            make_layer(thirds, movement="forward", active_step_count=16),
            make_layer(thirds_harm, movement="forward", active_step_count=12),
            empty_layer(),
            empty_layer(),
        ],
    )

    # Circle of fifths through the scale — “5ths heaven”
    # Diatonic 5ths: +4 scale degrees each step
    fifths_degrees = [0, 4, 1, 5, 2, 6, 3, 0]
    fifths_octs = [0, 0, 1, 1, 1, 1, 2, 2]
    fifths = [n(d, o, True, 78 + i * 3) for i, (d, o) in enumerate(zip(fifths_degrees, fifths_octs))]
    # Mirror descent for a 16-step ping feel
    fifths_16 = fifths + [
        n(3, 2, True, 100),
        n(6, 1, True, 94),
        n(2, 1, True, 90),
        n(5, 1, True, 86),
        n(1, 1, True, 82),
        n(4, 0, True, 80),
        n(0, 0, True, 84),
        n(4, 0, True, 78),
    ]
    presets["5ths Heaven"] = patch(
        "5ths Heaven — diatonic fifths spiral",
        root="D",
        mode="mixolydian",
        master_division=0.125,  # 1/8 — roomier, angelic
        min_octave=3,
        max_octave=7,
        layers=[make_layer(fifths_16, movement="ping_pong", active_step_count=16)],
    )

    # Open 5ths drone + sparkle 3rds (poly)
    open5 = [
        n(R, 0, True, 88),
        n(_5, 0, True, 82),
        n(R, 1, True, 92),
        n(_5, 1, True, 86),
        n(R, 0, True, 80),
        n(_5, 0, True, 76),
        n(R, 1, True, 90),
        n(_5, 0, True, 78),
    ]
    sparkle3 = [
        n(_3, 1, True, 70),
        n(_5, 1, True, 74),
        n(_7, 1, True, 78),
        n(_3, 2, True, 82),
        n(_5, 1, True, 72),
        n(_3, 1, True, 68),
    ]
    presets["Open Sky"] = patch(
        "Open Sky — 5ths bed + 3rds sparkle",
        root="C",
        mode="lydian",
        master_division=0.0625,
        polyphony=True,
        min_octave=2,
        max_octave=7,
        layers=[
            make_layer(open5, movement="forward", active_step_count=8),
            make_layer(sparkle3, movement="pendulum", active_step_count=6),
            empty_layer(),
            empty_layer(),
        ],
    )

    # Polyrhythm 3 against 4 (classic interlocking)
    pulse3 = [
        n(R, 0, True, 100),
        n(_5, 0, True, 70),
        n(_3, 0, True, 78),
    ]
    pulse4 = [
        n(R, 1, True, 72),
        n(_3, 1, True, 80),
        n(_5, 1, True, 76),
        n(R, 2, True, 88),
    ]
    presets["Poly 3v4"] = patch(
        "Poly 3v4 — interlocking triad vs tetrad",
        root="E",
        mode="dorian",
        master_division=0.0625,
        polyphony=True,
        min_octave=3,
        max_octave=6,
        layers=[
            make_layer(pulse3, movement="forward", active_step_count=3),
            make_layer(pulse4, movement="forward", active_step_count=4),
            empty_layer(),
            empty_layer(),
        ],
    )

    # 5 against 8 — melodic techno / progressive
    five = [
        n(R, 0, True, 94),
        n(_2, 0, True, 70),
        n(_4, 0, True, 82),
        n(_5, 0, True, 74),
        n(_7, 0, True, 88),
    ]
    eight = [
        n(R, 1, True, 68),
        n(_3, 1, False, 60),
        n(_5, 1, True, 78),
        n(_3, 1, True, 70),
        n(R, 2, True, 86),
        n(_5, 1, False, 60),
        n(_3, 1, True, 74),
        n(_7, 1, True, 80),
    ]
    presets["Poly 5v8"] = patch(
        "Poly 5v8 — melodic techno lattice",
        root="F#",
        mode="minor",
        master_division=0.0625,
        polyphony=True,
        min_octave=2,
        max_octave=6,
        layers=[
            make_layer(five, movement="forward", active_step_count=5),
            make_layer(eight, movement="forward", active_step_count=8),
            empty_layer(),
            empty_layer(),
        ],
    )

    # Triple stack 3×4×5 — long LCM (60 steps) before realign
    t3 = [n(R, 0, True, 96), n(_5, 0, True, 64), n(R, 0, False, 50)]
    t4 = [n(_3, 1, True, 70), n(_5, 1, True, 78), n(_7, 1, True, 74), n(_3, 1, False, 50)]
    t5 = [
        n(R, 2, True, 66),
        n(_2, 2, True, 70),
        n(_4, 2, True, 74),
        n(_5, 2, True, 70),
        n(_7, 2, True, 78),
    ]
    presets["Poly 3x4x5"] = patch(
        "Poly 3×4×5 — three-way polymeter",
        root="G",
        mode="dorian",
        master_division=0.0625,
        polyphony=True,
        min_octave=2,
        max_octave=6,
        layers=[
            make_layer(t3, movement="forward", active_step_count=3),
            make_layer(t4, movement="forward", active_step_count=4),
            make_layer(t5, movement="forward", active_step_count=5),
            empty_layer(),
        ],
    )

    # Soft / generative — trigger probability for airy trance pads feel
    soft = [
        n(R, 1, True, 70),
        n(_3, 1, True, 74),
        n(_5, 1, True, 80),
        n(R, 2, True, 86),
        n(_5, 1, True, 76),
        n(_3, 1, True, 72),
        n(_7, 1, True, 78),
        n(_5, 1, True, 70),
    ]
    soft_layer = make_layer(soft, movement="random_skip", active_step_count=8, random_skip_prob=0.18)
    # Arm light trigger prob on every other cell for haze
    for y, row in enumerate(soft_layer["cells"]):
        for x, c in enumerate(row):
            if (y * 4 + x) < 8 and (x + y) % 2 == 1:
                c["trigger_armed"] = True
                c["trigger_prob"] = 0.65
                c["jitter_armed"] = True
                c["jitter_amount"] = 0.25
    presets["Cloud Drift"] = patch(
        "Cloud Drift — soft skips + light jitter",
        root="Bb",
        mode="lydian",
        master_division=0.125,
        min_octave=3,
        max_octave=7,
        layers=[soft_layer],
    )

    # ── Abstract / experimental ──────────────────────────────────────────

    def arm_haze(layer: Dict[str, Any], steps: int, prob: float = 0.55, jitter: float = 0.35) -> None:
        for i in range(steps):
            c = layer["cells"][i // 4][i % 4]
            if not c["gate"]:
                continue
            c["trigger_armed"] = True
            c["trigger_prob"] = prob
            c["jitter_armed"] = True
            c["jitter_amount"] = jitter

    # One pitch — rhythm only (gates + velocity as the composition)
    one_note = [
        n(R, 1, True, 110),
        n(R, 1, False, 40),
        n(R, 1, True, 70),
        n(R, 1, True, 70),
        n(R, 1, False, 40),
        n(R, 1, False, 40),
        n(R, 1, True, 95),
        n(R, 1, False, 40),
        n(R, 1, True, 60),
        n(R, 1, True, 110),
        n(R, 1, False, 40),
        n(R, 1, True, 55),
        n(R, 1, False, 40),
        n(R, 1, False, 40),
        n(R, 1, True, 88),
        n(R, 1, False, 40),
    ]
    presets["One Note Universe"] = patch(
        "One Note Universe — pitch frozen, rhythm speaks",
        root="C",
        mode="chromatic",
        master_division=0.0625,
        min_octave=3,
        max_octave=5,
        layers=[make_layer(one_note, movement="forward", active_step_count=16)],
    )

    # Euclidean-ish 5-in-16 rain (Bjorklund-style spacing via gates)
    euclid_hits = {0, 3, 6, 10, 13}
    euclid = [
        n((i * 2) % 7, i // 8, i in euclid_hits, 70 + (i % 5) * 6) for i in range(16)
    ]
    presets["Euclid Rain"] = patch(
        "Euclid Rain — 5 hits scattered across 16",
        root="D",
        mode="pentatonic_minor",
        master_division=0.0625,
        min_octave=3,
        max_octave=6,
        layers=[make_layer(euclid, movement="forward", active_step_count=16)],
    )

    # Morse-like long/short: dash=gate+loud, dot=gate+soft, space=rest
    # Pattern approximates "SOS" rhythm as abstract pulse (.. . / --- / .. .)
    morse = [
        n(R, 1, True, 60),
        n(R, 1, False, 40),
        n(R, 1, True, 60),
        n(R, 1, False, 40),
        n(R, 1, True, 60),
        n(R, 1, False, 40),
        n(R, 1, False, 40),
        n(_5, 1, True, 110),
        n(_5, 1, True, 110),
        n(_5, 1, True, 110),
        n(R, 1, False, 40),
        n(R, 1, False, 40),
        n(R, 1, True, 60),
        n(R, 1, False, 40),
        n(R, 1, True, 60),
        n(R, 1, False, 40),
    ]
    presets["Signal Fog"] = patch(
        "Signal Fog — Morse-like long/short voids",
        root="Eb",
        mode="minor",
        master_division=0.125,
        min_octave=2,
        max_octave=5,
        layers=[make_layer(morse, movement="forward", active_step_count=16)],
    )

    # Prime-number polymeter swarm (5×7×11 — almost never realigns)
    p5 = [n(R, 0, True, 100), n(_2, 0, False, 40), n(_4, 0, True, 72), n(_5, 0, False, 40), n(_7, 0, True, 84)]
    p7 = [
        n(_3, 1, True, 66),
        n(_3, 1, False, 40),
        n(_5, 1, True, 70),
        n(_7, 1, False, 40),
        n(R, 2, True, 78),
        n(_2, 2, False, 40),
        n(_4, 2, True, 74),
    ]
    p11 = [
        n(R, 2, True, 50 + i * 3) if i % 3 == 0 else n((i % 7), 2, False, 40) for i in range(11)
    ]
    presets["Prime Swarm"] = patch(
        "Prime Swarm — 5×7×11 never quite meets",
        root="F#",
        mode="locrian",
        master_division=0.0625,
        polyphony=True,
        min_octave=2,
        max_octave=6,
        layers=[
            make_layer(p5, movement="forward", active_step_count=5),
            make_layer(p7, movement="forward", active_step_count=7),
            make_layer(p11, movement="forward", active_step_count=11),
            empty_layer(),
        ],
    )

    # Chaos — random path + heavy skip + jitter haze
    chaos_notes = [n((i * 3 + 1) % 7, i % 3, True, 60 + (i * 7) % 50) for i in range(16)]
    chaos_layer = make_layer(chaos_notes, movement="random", active_step_count=16)
    arm_haze(chaos_layer, 16, prob=0.45, jitter=0.7)
    chaos_b = make_layer(
        [n((i * 5) % 7, (i // 4) % 2, i % 2 == 0, 55 + i * 2) for i in range(12)],
        movement="random_skip",
        active_step_count=12,
        random_skip_prob=0.35,
    )
    arm_haze(chaos_b, 12, prob=0.4, jitter=0.55)
    presets["Glass Shatter"] = patch(
        "Glass Shatter — random walks + probability fog",
        root="B",
        mode="harmonic_minor",
        master_division=0.0625,
        polyphony=True,
        min_octave=3,
        max_octave=7,
        layers=[chaos_layer, chaos_b, empty_layer(), empty_layer()],
    )

    # Negative space — mostly silence, rare events
    void_notes = [n(R, 2, False, 40) for _ in range(16)]
    for idx, deg, oct_, vel in ((0, R, 1, 100), (7, _5, 2, 70), (11, _3, 1, 55), (15, R, 3, 90)):
        void_notes[idx] = n(deg, oct_, True, vel)
    void_layer = make_layer(void_notes, movement="forward", active_step_count=16)
    arm_haze(void_layer, 16, prob=0.7, jitter=0.2)
    presets["Staircase Void"] = patch(
        "Staircase Void — composition of silence",
        root="A",
        mode="phrygian",
        master_division=0.25,
        min_octave=2,
        max_octave=7,
        layers=[void_layer],
    )

    # Binary clock — gates as bit pattern of an interesting byte stream
    bits = [1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1]  # abstract bitfield
    binary = [n((i % 5), 1 if bits[i] else 0, bool(bits[i]), 50 + bits[i] * 55) for i in range(16)]
    presets["Binary Clock"] = patch(
        "Binary Clock — gates as a bitfield",
        root="G",
        mode="pentatonic",
        master_division=0.0625,
        min_octave=3,
        max_octave=6,
        layers=[make_layer(binary, movement="forward", active_step_count=16)],
    )

    # Mirror — layer A forward, layer B reverse same contour
    mirror_a = [
        n(R, 0, True, 88),
        n(_2, 0, True, 76),
        n(_4, 0, True, 82),
        n(_5, 0, True, 90),
        n(_7, 0, True, 96),
        n(_5, 1, True, 100),
        n(_4, 1, True, 86),
        n(_2, 1, True, 78),
    ]
    mirror_b = list(reversed(mirror_a))
    presets["Mirror Lake"] = patch(
        "Mirror Lake — forward meets its reverse",
        root="C",
        mode="lydian",
        master_division=0.125,
        polyphony=True,
        min_octave=3,
        max_octave=6,
        layers=[
            make_layer(mirror_a, movement="forward", active_step_count=8),
            make_layer(mirror_b, movement="forward", active_step_count=8),
            empty_layer(),
            empty_layer(),
        ],
    )

    # Heartbeat — ba-BUM rest rest
    heart = [
        n(R, 0, True, 70),
        n(R, 0, True, 115),
        n(R, 0, False, 40),
        n(R, 0, False, 40),
        n(R, 0, True, 70),
        n(R, 0, True, 115),
        n(R, 0, False, 40),
        n(R, 0, False, 40),
    ]
    breath = [
        n(_5, 1, True, 45),
        n(_5, 1, False, 40),
        n(_3, 1, True, 40),
        n(_3, 1, False, 40),
        n(R, 2, True, 50),
        n(R, 2, False, 40),
    ]
    presets["Heartbeat"] = patch(
        "Heartbeat — ba-BUM with a thin breath",
        root="D",
        mode="minor",
        master_division=0.125,
        polyphony=True,
        min_octave=1,
        max_octave=5,
        layers=[
            make_layer(heart, movement="forward", active_step_count=8),
            make_layer(breath, movement="pendulum", active_step_count=6),
            empty_layer(),
            empty_layer(),
        ],
    )

    # Insect choir — fast, tiny motion, high skip
    insect = [n((i % 3), 2, True, 40 + (i % 4) * 8) for i in range(16)]
    insect_layer = make_layer(insect, movement="random_skip", active_step_count=16, random_skip_prob=0.4)
    arm_haze(insect_layer, 16, prob=0.5, jitter=0.15)
    presets["Insect Choir"] = patch(
        "Insect Choir — fast micro-motion swarm",
        root="E",
        mode="pentatonic_minor",
        master_division=0.03125,  # 1/32
        min_octave=4,
        max_octave=7,
        layers=[insect_layer],
    )

    # Slow orbit — glacial drone pendulum across a wide sky
    orbit = [
        n(R, 0, True, 80),
        n(_5, 0, True, 70),
        n(R, 1, True, 85),
        n(_5, 1, True, 75),
        n(R, 2, True, 90),
        n(_5, 2, True, 78),
        n(R, 3, True, 95),
        n(_5, 2, True, 70),
    ]
    presets["Slow Orbit"] = patch(
        "Slow Orbit — glacial fifth pendulum",
        root="F",
        mode="lydian",
        master_division=0.25,  # quarter notes
        min_octave=1,
        max_octave=7,
        layers=[make_layer(orbit, movement="pendulum", active_step_count=8)],
    )

    # Broken clock — intentionally awkward lengths + rests
    broken = [
        n(R, 1, True, 100),
        n(R, 1, False, 40),
        n(_3, 1, True, 60),
        n(_3, 1, True, 60),
        n(_3, 1, False, 40),
        n(_5, 1, True, 90),
        n(_5, 1, False, 40),
        n(_5, 1, False, 40),
        n(_7, 1, True, 50),
        n(R, 2, True, 110),
        n(R, 2, False, 40),
        n(_2, 1, True, 55),
        n(_2, 1, False, 40),
        n(_4, 1, True, 70),
        n(_4, 1, True, 70),
        n(_4, 1, False, 40),
    ]
    presets["Broken Clock"] = patch(
        "Broken Clock — awkward meter, wrong accents",
        root="Bb",
        mode="dorian",
        master_division=0.0625,
        min_octave=2,
        max_octave=5,
        layers=[make_layer(broken, movement="forward", active_step_count=13)],
    )

    # Static field — near-unison texture, probability as the weather
    static = [n(R, 1, True, 50 + (i % 7) * 5) for i in range(16)]
    static_layer = make_layer(static, movement="forward", active_step_count=16)
    arm_haze(static_layer, 16, prob=0.35, jitter=0.85)
    presets["Static Field"] = patch(
        "Static Field — one pitch, weather is probability",
        root="C",
        mode="chromatic",
        master_division=0.0625,
        min_octave=3,
        max_octave=4,
        layers=[static_layer],
    )

    # Whale call — wide leaps, slow, pentatonic minor
    whale = [
        n(R, 0, True, 90),
        n(R, 0, False, 40),
        n(R, 0, False, 40),
        n(_5, 2, True, 70),
        n(_5, 2, False, 40),
        n(_3, 1, True, 80),
        n(_3, 1, False, 40),
        n(R, 3, True, 60),
        n(R, 3, False, 40),
        n(R, 3, False, 40),
        n(_4, 0, True, 75),
        n(_4, 0, False, 40),
        n(R, 1, True, 85),
        n(R, 1, False, 40),
        n(_5, 1, True, 65),
        n(_5, 1, False, 40),
    ]
    presets["Whale Call"] = patch(
        "Whale Call — wide slow leaps across fog",
        root="D",
        mode="pentatonic_minor",
        master_division=0.25,
        min_octave=1,
        max_octave=6,
        layers=[make_layer(whale, movement="ping_pong", active_step_count=16)],
    )

    # Cathedral fog — four sparse layers, different primes, very slow
    cat_a = [n(R, 0, True, 70), n(R, 0, False, 40), n(_5, 0, True, 55), n(_5, 0, False, 40)]
    cat_b = [n(_3, 1, True, 50), n(_3, 1, False, 40), n(_3, 1, False, 40)]
    cat_c = [n(R, 2, True, 45), n(R, 2, False, 40), n(R, 2, False, 40), n(_7, 1, True, 40), n(_7, 1, False, 40)]
    cat_d = [n(_5, 2, True, 40), n(_5, 2, False, 40)]
    presets["Cathedral Fog"] = patch(
        "Cathedral Fog — four slow ghosts in a hall",
        root="G",
        mode="lydian",
        master_division=0.25,
        polyphony=True,
        min_octave=1,
        max_octave=6,
        layers=[
            make_layer(cat_a, movement="pendulum", active_step_count=4),
            make_layer(cat_b, movement="forward", active_step_count=3),
            make_layer(cat_c, movement="reverse", active_step_count=5),
            make_layer(cat_d, movement="ping_pong", active_step_count=2),
        ],
    )

    # Fibonacci bloom — degrees & step lengths from fib-ish series
    fib_deg = [0, 1, 1, 2, 3, 5, 8, 13]  # will wrap via % 7 and oct
    fib_notes = []
    for i, f in enumerate(fib_deg):
        fib_notes.append(n(f % 7, f // 7, True, 70 + i * 5))
    fib_notes += [
        n(5 % 7, 0, False, 40),
        n(3, 0, True, 80),
        n(2, 0, True, 75),
        n(1, 0, True, 70),
        n(1, 0, True, 65),
        n(0, 0, True, 90),
        n(0, 0, False, 40),
        n(0, 1, True, 85),
    ]
    presets["Fibonacci Bloom"] = patch(
        "Fibonacci Bloom — sequence as melody scaffold",
        root="A",
        mode="major",
        master_division=0.125,
        min_octave=3,
        max_octave=6,
        layers=[make_layer(fib_notes, movement="ping_pong", active_step_count=16)],
    )

    # Phrygian knife — dark short stabs
    knife = [
        n(R, 1, True, 120),
        n(R, 1, False, 40),
        n(_2, 1, True, 90),  # b2 colour in phrygian
        n(_2, 1, False, 40),
        n(R, 1, True, 110),
        n(_5, 0, True, 70),
        n(_5, 0, False, 40),
        n(R, 0, True, 100),
    ]
    presets["Phrygian Knife"] = patch(
        "Phrygian Knife — dark short stabs",
        root="E",
        mode="phrygian",
        master_division=0.0625,
        min_octave=2,
        max_octave=5,
        layers=[make_layer(knife, movement="forward", active_step_count=8)],
    )

    return presets


def main() -> None:
    out_dir = Path(__file__).resolve().parent
    presets = build_presets()
    for name, data in presets.items():
        path = out_dir / f"{name}.json"
        path.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
        print(f"wrote {path.name}")
    print(f"done — {len(presets)} presets")


if __name__ == "__main__":
    main()
