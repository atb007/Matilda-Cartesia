#!/usr/bin/env python3
"""
Run a .teleiso script → isobar Timeline → MIDI (IAC / GarageBand).

Usage:
  python3 live_teleiso.py examples/legato_euclidean.teleiso
  python3 live_teleiso.py examples/euclidean_c_major.teleiso --dry-run
  USE_VIRTUAL=1 python3 live_teleiso.py examples/arp_minor.teleiso

Teleiso lines compile to isobar patterns. Isobar is unchanged underneath.
Ctrl+C to stop.
"""

from __future__ import annotations

import argparse
import logging
import os
import signal
import sys

from isobar.io.midi.output import MidiOutputDevice

from teleiso import compile_file, preview


def open_midi_out(patch):
    if patch.use_virtual_midi or os.environ.get("USE_VIRTUAL") == "1":
        return MidiOutputDevice(device_name="Isobar Live Arp", virtual=True)
    return MidiOutputDevice(device_name=patch.midi_port)


def dry_run(patch) -> None:
    print(f"Compiled: {patch.title}")
    print(f"  tempo={patch.tempo}  key={patch.key}  gate={patch.gate}")
    sched = patch.schedule_dict()
    for name in ("degree", "note", "active", "duration", "amplitude"):
        pat = sched.get(name)
        if pat is None:
            continue
        if isinstance(pat, (int, float)):
            print(f"  {name}: {pat}")
        else:
            print(f"  {name}: {preview(pat, 12)}")


def main():
    parser = argparse.ArgumentParser(description="Run Teleiso → isobar → MIDI")
    parser.add_argument("script", help="path to .teleiso file")
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="compile and preview values, do not open MIDI",
    )
    args = parser.parse_args()

    logging.basicConfig(level=logging.INFO, format="[%(asctime)s] %(message)s")

    patch = compile_file(args.script)

    if args.dry_run:
        dry_run(patch)
        return

    from teleiso.compiler import run_timeline

    midi_out = open_midi_out(patch)
    print(patch.title)
    print(f"MIDI → {patch.midi_port} (or USE_VIRTUAL=1)")
    print("GarageBand: Record arm + Play. Ctrl+C to stop.\n")

    timeline = run_timeline(patch, output_device=midi_out)

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
