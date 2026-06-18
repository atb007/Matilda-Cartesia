#pragma once

#include <cmath>

namespace matilda {

/** Step length as a fraction of one bar (BlueARP sync parity). */
struct ClockDivision {
    const char* label;
    double masterDivision;
};

inline constexpr ClockDivision kClockDivisions[] = {
    {"1/64", 1.0 / 64.0},
    {"1/48", 1.0 / 48.0},
    {"1/32", 1.0 / 32.0},
    {"1/24", 1.0 / 24.0},
    {"1/16", 1.0 / 16.0},
    {"1/12", 1.0 / 12.0},
    {"1/8", 1.0 / 8.0},
    {"1/6", 1.0 / 6.0},
    {"1/4", 1.0 / 4.0},
    {"3/64", 3.0 / 64.0},
    {"3/32", 3.0 / 32.0},
    {"3/16", 3.0 / 16.0},
    {"3/8", 3.0 / 8.0},
};

inline constexpr int kClockDivisionCount = static_cast<int>(sizeof(kClockDivisions) / sizeof(kClockDivisions[0]));

inline int indexForMasterDivision(double division) {
    for (int i = 0; i < kClockDivisionCount; ++i) {
        if (std::abs(kClockDivisions[static_cast<size_t>(i)].masterDivision - division) < 0.0001)
            return i;
    }

    int best = 0;
    double bestDist = std::abs(kClockDivisions[0].masterDivision - division);
    for (int i = 1; i < kClockDivisionCount; ++i) {
        const double dist = std::abs(kClockDivisions[static_cast<size_t>(i)].masterDivision - division);
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    return best;
}

} // namespace matilda
