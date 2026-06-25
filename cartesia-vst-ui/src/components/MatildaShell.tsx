import { Grid4x4 } from "./Grid4x4";
import { KNOB_VARIANT_ORDER } from "./knobColors";
import { LayerMiniGrid } from "./LayerMiniGrid";
import { MovementMenu } from "./MovementMenu";
import { ScalePanel } from "./ScalePanel";
import { SHELL_FRAME_H, SHELL_H, SHELL_W } from "../shellLayout";
import { ShellFrameOverlay } from "./ShellFrameOverlay";
import { ShellGlassBedding } from "./ShellGlassBedding";
import { TransportChrome } from "./TransportChrome";
import { useMatildaEngine } from "../hooks/useMatildaEngine";

/**
 * M8 + M9 — Control-panel shell wired to Cartesia engine.
 */

const POS = {
  scalePanel: { left: 118, top: 200 },
  layerOverview: { left: 628.879, top: 218.21 },
  movement: { left: 675.18, top: 667.796 },
  grid: { left: 629, top: 818.945 },
  transport: { left: 103.218, top: 887.449 },
} as const;

const GRID_CELL_W = 122.35546875;
const GRID_CELL_H = 140.9765625;
const GRID_COL_GAP = 38.64453125;
const GRID_ROW_GAP = 17.078125;

type Props = {
  scale?: number;
  embedded?: boolean;
};

export function MatildaShell({ scale = 0.78, embedded = false }: Props) {
  const engine = useMatildaEngine();
  const {
    patch,
    playing,
    playheadStep,
    playingLayer,
    movement,
    scaleId,
    tonic,
    minOctave,
    maxOctave,
    clockDivision,
    playMode,
    dawSync,
    setPlaying,
    setSelectedLayer,
    toggleLayer,
    setMovement,
    setScaleId,
    setTonic,
    setMinOctave,
    setMaxOctave,
    setClockDivision,
    setPlayMode,
    setDawSync,
    patchCell,
    layerCellsFlat,
  } = engine;

  const selectedLayer = patch.selectedLayer;
  const variant = KNOB_VARIANT_ORDER[selectedLayer];
  const cells = layerCellsFlat(selectedLayer);
  const activeLayers = patch.layers.map(l => l.active);

  const mainPlayhead =
    playing && playingLayer === selectedLayer ? playheadStep : -1;

  const s = scale;
  const shellH = embedded ? SHELL_H : SHELL_FRAME_H;

  return (
    <div
      className="select-none"
      style={{
        position: "relative",
        width: SHELL_W * s,
        height: shellH * s,
      }}
    >
      <div
        style={{
          position: "relative",
          width: SHELL_W,
          height: shellH,
          overflow: "visible",
          isolation: "isolate",
          transform: `scale(${s})`,
          transformOrigin: "top left",
        }}
      >
        <ShellGlassBedding useRaster={!embedded} />
        <ShellFrameOverlay />

        <div style={{ position: "absolute", inset: 0, zIndex: 3 }}>
          <div style={{ position: "absolute", left: POS.scalePanel.left, top: POS.scalePanel.top }}>
            <ScalePanel
              scaleId={scaleId}
              onScaleChange={setScaleId}
              tonic={tonic}
              onTonicChange={setTonic}
              minOctave={minOctave}
              maxOctave={maxOctave}
              onMinOctaveChange={setMinOctave}
              onMaxOctaveChange={setMaxOctave}
            />
          </div>

          <div style={{ position: "absolute", left: POS.layerOverview.left, top: POS.layerOverview.top }}>
            <LayerMiniGrid
              activeLayers={activeLayers}
              selectedLayer={selectedLayer}
              playheadStep={playing ? playheadStep : -1}
              playingLayer={playing ? playingLayer : -1}
              layerCells={patch.layers.map((_, i) => layerCellsFlat(i))}
              onToggleLayer={toggleLayer}
              onSelectLayer={setSelectedLayer}
            />
          </div>

          <div style={{ position: "absolute", left: POS.movement.left, top: POS.movement.top }}>
            <MovementMenu value={movement} onChange={setMovement} />
          </div>

          <div style={{ position: "absolute", left: POS.grid.left, top: POS.grid.top }}>
            <Grid4x4
              variant={variant}
              cellWidth={GRID_CELL_W}
              cellHeight={GRID_CELL_H}
              colGap={GRID_COL_GAP}
              rowGap={GRID_ROW_GAP}
              playheadStep={mainPlayhead}
              cells={cells}
              patch={patch}
              onCellChange={(index, cellPatch) => patchCell(selectedLayer, index, cellPatch)}
            />
          </div>

          <div style={{ position: "absolute", left: POS.transport.left, top: POS.transport.top }}>
            <TransportChrome
              playing={playing}
              onPlayingChange={setPlaying}
              playMode={playMode}
              onPlayModeChange={setPlayMode}
              clockDivision={clockDivision}
              onClockDivisionChange={setClockDivision}
              dawSync={dawSync}
              onDawSyncChange={setDawSync}
            />
          </div>
        </div>
      </div>
    </div>
  );
}
