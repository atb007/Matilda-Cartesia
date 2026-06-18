#!/usr/bin/env python3
"""
Matilda (Cartesia v2) grid prototype → isobar Timeline → MIDI.

Usage:
  python3 live_cartesia.py --dry-run
  python3 live_cartesia.py matilda/presets/default.layer1.json --dry-run
  python3 live_cartesia.py matilda/presets/default.layer1.json
"""

from __future__ import annotations

import argparse

from cartesia.model import demo_patch, load_patch
from cartesia.runner import preview_patch, run_patch, open_midi_out


def main():
    parser = argparse.ArgumentParser(description="Cartesia grid → MIDI prototype")
    parser.add_argument("preset", nargs="?", help="JSON preset path (optional)")
    parser.add_argument("--dry-run", action="store_true", help="preview steps, no MIDI")
    args = parser.parse_args()

    if args.preset:
        patch = load_patch(args.preset)
    else:
        patch = demo_patch()

    if args.dry_run:
        preview_patch(patch, steps=24)
        return

    midi = open_midi_out(patch)
    print(patch.title)
    print("Matilda: path movement + sequential layers")
    print("MIDI → IAC (USE_VIRTUAL=1 for virtual port). Ctrl+C to stop.\n")
    timeline = run_patch(patch, output_device=midi)

    import signal

    def stop(_sig, _frame):
        timeline.stop()
        sys.exit(0)

    signal.signal(signal.SIGINT, stop)
    timeline.run()


if __name__ == "__main__":
    main()
