/** Row-major / mini-grid index helpers shared by UI grids. */

export { type Cell } from "./engine";

/** Mini-grid column-major index → main grid row-major step. */
export function rowMajorFromMiniGridIndex(miniIndex: number): number {
  const col = Math.floor(miniIndex / 4);
  const row = miniIndex % 4;
  return row * 4 + col;
}
