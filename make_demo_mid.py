#!/usr/bin/env python3
"""Write a short arpeggio MIDI file to the Desktop (no IAC / no hardware needed)."""

from isobar import *

key = Key("A", "minor")
arpeggio = PSeries(0, 2, 4)
arpeggio = PDegree(arpeggio, key) + 60
arpeggio = PPingPong(arpeggio)
arpeggio = PLoop(arpeggio)

import os

out_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "demo.mid")
output = MidiFileOutputDevice(out_path)
timeline = Timeline(120, output_device=output)
timeline.stop_when_done = True

timeline.schedule(
    {
        "note": arpeggio,
        "duration": 0.25,
        "amplitude": PSequence([90, 70, 60, 70]),
    },
    count=32,
)

timeline.run()
output.write()
print(f"Created {out_path}")
