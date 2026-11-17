#pragma once

#include <array>
#include <cstddef>
#include "schreihuhn/core/math_types.h"

namespace schreihuhn::world {

struct TrajectorySample {
    float x = 0.0f;
    float y = 0.0f; // relative to starting ground level (negative = up)
};

struct JumpSimResult {
    float distance_px = 0.0f;
    float peak_height_px = 0.0f;
    std::array<TrajectorySample, 128> samples{};
    std::size_t sample_count = 0;
};

class SolvabilityChecker {
public:
    // Simulates a jump from ground y=0 to target ground y=delta_y (negative = higher)
    // using exact Physics equations and K_PHYS_DT.
    static JumpSimResult simulateJump(float delta_y, bool with_flap);

    // Returns conservative max tile-aligned gap width (multiple of 8 px)
    // obeying FR-MP-36 (<= 85% without flap, <= 80% with flap).
    static float maxGapPx(float delta_y, bool with_flap);

    // Checks whether an obstacle of given height is surmountable (jump or jump+flap).
    static bool canClearObstacle(float obstacle_h, bool allow_flap);

    // Checks whether an overhead obstacle leaves enough clearance to walk underneath.
    static bool canWalkUnder(float clearance_px);
};

} // namespace schreihuhn::world
