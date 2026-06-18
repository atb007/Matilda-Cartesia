"""Isobar bridge for Matilda / Cartesia engine."""

from __future__ import annotations

import os
import signal
import sys
from typing import Tuple

from isobar import Pattern, Timeline
from isobar.io.midi.output import MidiOutputDevice

from cartesia.engine import CartesiaEngine, StepEvent
from cartesia.model import Patch, demo_patch, load_patch


class PCartesiaField(Pattern):
    """Reads shared engine cache so degree/active/velocity stay in sync."""

    def __init__(self, engine: CartesiaEngine, field: str):
        self.engine = engine
        self.field = field

    def __repr__(self):
        return f"PCartesiaField({self.field!r})"

    def __next__(self):
        event = self.engine.current()
        value = None
        if self.field == "degree":
            value = event.degree
        elif self.field == "velocity":
            value = event.velocity if event.fired else None
        elif self.field == "active":
            value = 1 if event.fired and event.degree is not None else None
        else:
            raise ValueError(f"unknown field {self.field!r}")
        self.engine.mark_field_read()
        return value


def open_midi_out(_patch: Patch):
    if os.environ.get("USE_VIRTUAL") == "1":
        return MidiOutputDevice(device_name="Isobar Live Arp", virtual=True)
    return MidiOutputDevice(device_name="IAC Driver Bus 1")


def build_timeline(patch: Patch, output_device=None) -> Tuple[Timeline, CartesiaEngine]:
    engine = CartesiaEngine(patch)
    engine.reset()

    timeline = Timeline(120, output_device=output_device)
    base_octave = patch.min_octave + 2
    timeline.schedule(
        {
            "degree": PCartesiaField(engine, "degree"),
            "key": engine.key,
            "octave": base_octave,
            "duration": patch.master_division,
            "gate": 0.9,
            "amplitude": PCartesiaField(engine, "velocity"),
            "active": PCartesiaField(engine, "active"),
        }
    )
    return timeline, engine


def preview_patch(patch: Patch, steps: int = 16) -> None:
    engine = CartesiaEngine(patch)
    engine.reset()
    layer = patch.layers[patch.selected_layer]
    print(f"Patch: {patch.title} (v{patch.version})")
    print(f"  selected layer Z{patch.selected_layer + 1}  movement={layer.movement.value}")
    print(f"  active layers: {[i + 1 for i, L in enumerate(patch.layers) if L.active]}")
    print(f"  master={patch.master_division} beat  scale={patch.root} {patch.mode}")
    print(f"  playhead start {engine.playhead()}")
    print("\n  step  x y z  deg  vel  fire")
    for i, event in enumerate(engine.preview_steps(steps), start=1):
        deg = event.degree if event.degree is not None else "—"
        vel = event.velocity if event.velocity is not None else "—"
        mark = "●" if event.fired else "·"
        print(f"  {i:02d}   {event.x} {event.y} {event.z}   {deg!s:>3}  {vel!s:>3}  {mark}")


def run_patch(patch: Patch, output_device=None) -> Timeline:
    timeline, _ = build_timeline(patch, output_device=output_device)
    return timeline


def run_patch_file(path: str, dry_run: bool = False) -> None:
    patch = load_patch(path)
    if dry_run:
        preview_patch(patch)
        return

    midi = open_midi_out(patch)
    print(patch.title)
    print("Matilda grid engine → MIDI (IAC or USE_VIRTUAL=1)")
    print("GarageBand: Record arm + Play @ 120 BPM. Ctrl+C to stop.\n")
    timeline = run_patch(patch, output_device=midi)

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


def run_demo(dry_run: bool = False) -> None:
    patch = demo_patch()
    if dry_run:
        preview_patch(patch)
        return
    midi = open_midi_out(patch)
    timeline = run_patch(patch, output_device=midi)

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()
