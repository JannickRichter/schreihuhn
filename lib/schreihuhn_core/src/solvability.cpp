#include "schreihuhn/world/solvability.h"
#include "schreihuhn/config/tuning.h"

#include <algorithm>
#include <cmath>

namespace schreihuhn::world {

JumpSimResult SolvabilityChecker::simulateJump(float delta_y, bool with_flap) {
    JumpSimResult res{};
    constexpr float dt = config::K_PHYS_DT;

    float x = 0.0f;
    float y = 0.0f; // bottom of hitbox relative to start ground (0 = start ground)
    float vx = config::K_AIR_VX;
    float vy = -config::K_JUMP_VY;
    float stamina = config::K_FLAP_STAMINA;
    bool is_flapping = false;
    float min_y = 0.0f;

    for (int tick = 0; tick < 600; ++tick) {
        if (tick > 0) {
            if (with_flap && stamina > 0.0f) {
                vx = config::K_FLAP_VX;
                is_flapping = true;
                if (vy >= 0.0f) {
                    stamina = std::max(0.0f, stamina - dt);
                    if (stamina > 0.0f) {
                        vy = std::min(vy, config::K_FLAP_MAX_FALL);
                    } else {
                        is_flapping = false;
                        vx = config::K_AIR_VX;
                    }
                }
            } else {
                if (is_flapping) {
                    vx = config::K_AIR_VX;
                }
                is_flapping = false;
            }
        }

        vy = std::min(vy + config::K_GRAVITY * dt, config::K_MAX_FALL);
        const float prev_y = y;
        x += vx * dt;
        y += vy * dt;
        if (y < min_y) {
            min_y = y;
        }

        if ((tick % 2) == 0 && res.sample_count < res.samples.size()) {
            res.samples[res.sample_count++] = TrajectorySample{x, y};
        }

        // Crossed target ground level while descending
        if (vy >= 0.0f && prev_y <= delta_y + 0.5f && y >= delta_y) {
            res.distance_px = x;
            break;
        }
        // Fallback if target is unreachable (above peak)
        if (y > delta_y + 64.0f) {
            res.distance_px = 0.0f;
            break;
        }
    }

    res.peak_height_px = -min_y;
    return res;
}

float SolvabilityChecker::maxGapPx(float delta_y, bool with_flap) {
    const JumpSimResult sim = simulateJump(delta_y, with_flap);
    const float factor = with_flap ? 0.80f : 0.85f;
    const float raw_max = sim.distance_px * factor;
    const float aligned = std::floor(raw_max / config::K_TILE_SIZE) * config::K_TILE_SIZE;
    return std::max(0.0f, aligned);
}

bool SolvabilityChecker::canClearObstacle(float obstacle_h, bool allow_flap) {
    const JumpSimResult sim = simulateJump(0.0f, allow_flap);
    return sim.peak_height_px >= obstacle_h + 2.0f;
}

bool SolvabilityChecker::canWalkUnder(float clearance_px) {
    return clearance_px >= config::K_HITBOX_H + 4.0f;
}

} // namespace schreihuhn::world
