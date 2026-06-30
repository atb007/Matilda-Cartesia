#pragma once

/**
 * Layer overview mini-grid geometry — ported from cartesia-vst-ui LayerMiniGrid.tsx
 * (Figma 4957:102758).
 */
namespace matilda::minigrid {

inline constexpr float kBaseW = 609.565f;
inline constexpr float kBaseH = 301.43f;
inline constexpr float kCell = 30.f;
inline constexpr float kInactiveYNudge = -2.5f;

inline constexpr float kFrameH = kBaseW * (241.f / 518.f);
inline constexpr float kFrameTop = kBaseH * 0.5f + 4.29f - kFrameH * 0.5f;

struct CellPos {
    float leftPct;
    float topPx;
};

inline constexpr CellPos kLayer0Cells[] = {
    {4.52f, 47.f},   {4.45f, 100.f},  {4.45f, 156.f},  {4.45f, 211.f},
    {10.19f, 69.f},  {10.19f, 120.f}, {10.19f, 175.f}, {10.19f, 231.f},
    {15.44f, 90.f},  {15.44f, 140.f}, {15.44f, 198.f}, {15.44f, 250.f},
    {21.18f, 110.f}, {21.18f, 165.f}, {21.18f, 218.f}, {21.35f, 269.f},
};

inline constexpr CellPos kLayer1Cells[] = {
    {27.86f, 47.34f},  {27.86f, 99.91f},  {27.86f, 156.17f}, {27.86f, 211.49f},
    {33.43f, 70.4f},   {33.43f, 122.96f}, {33.43f, 179.22f}, {33.43f, 234.56f},
    {39.01f, 86.07f},  {39.01f, 138.64f}, {39.01f, 194.9f},  {39.01f, 250.24f},
    {43.98f, 107.28f}, {43.98f, 159.85f}, {43.98f, 216.1f},  {43.98f, 271.43f},
};

inline constexpr CellPos kLayer2Cells[] = {
    {51.96f, 47.34f},  {51.96f, 100.83f}, {51.96f, 158.01f}, {51.96f, 211.49f},
    {57.38f, 70.4f},   {57.38f, 121.13f}, {57.38f, 178.3f},  {57.38f, 231.78f},
    {62.2f, 86.07f},   {62.2f, 139.57f},  {62.2f, 196.74f}, {62.2f, 250.24f},
    {68.07f, 107.28f}, {68.07f, 160.78f}, {68.07f, 217.95f}, {68.07f, 271.43f},
};

inline constexpr CellPos kLayer3Cells[] = {
    {75.45f, 47.34f},  {75.45f, 100.83f}, {75.75f, 158.01f}, {75.75f, 211.49f},
    {80.87f, 70.4f},   {80.87f, 121.13f}, {80.87f, 178.3f},  {80.87f, 234.56f},
    {86.3f, 87.92f},   {86.3f, 139.57f},  {86.3f, 198.59f},  {86.3f, 250.24f},
    {91.72f, 107.28f}, {91.72f, 160.78f}, {91.72f, 217.95f}, {91.72f, 271.43f},
};

inline constexpr CellPos kToggles[] = {
    {12.49f, 6.f},
    {35.46f, 3.f},
    {58.75f, 3.f},
    {82.05f, 3.f},
};

inline constexpr int kCellsPerLayer = 16;
inline constexpr int kLayerCount = 4;

inline const CellPos* layerCells(int layer) {
    switch (layer) {
        case 1:  return kLayer1Cells;
        case 2:  return kLayer2Cells;
        case 3:  return kLayer3Cells;
        default: return kLayer0Cells;
    }
}

/** Main 4×4 row-major step → mini-grid column-major cell index. */
inline int miniGridIndexFromRowMajorStep(int step) {
    const int row = step / 4;
    const int col = step % 4;
    return col * 4 + row;
}

/** Mini-grid column-major index → main grid row-major step. */
inline int rowMajorFromMiniGridIndex(int miniIndex) {
    const int col = miniIndex / 4;
    const int row = miniIndex % 4;
    return row * 4 + col;
}

} // namespace matilda::minigrid
