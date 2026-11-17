#pragma once

#include <cstdint>
#include "schreihuhn/config/tuning.h"

namespace schreihuhn::game {

enum class PlayerAnim : uint8_t {
    Idle = 0,
    Walk = 1,
    Jump = 2,
    Flap = 3,
    Fall = 4
};

struct PlayerState {
    double world_x    = config::K_START_X;
    float  y          = config::K_LEVEL_0 - config::K_HITBOX_H;
    float  vx         = 0.0f;
    float  vy         = 0.0f;
    bool   on_ground  = true;
    bool   is_flapping = false;
    float  stamina    = config::K_FLAP_STAMINA;
    float  coyote_timer = config::K_COYOTE;
    PlayerAnim anim   = PlayerAnim::Idle;
    float  anim_timer = 0.0f;
    uint8_t anim_frame = 0;
    float  perk_timer = 0.0f; // > 0 when white-packet flight perk is active

    void reset() {
        world_x      = config::K_START_X;
        y            = config::K_LEVEL_0 - config::K_HITBOX_H;
        vx           = 0.0f;
        vy           = 0.0f;
        on_ground    = true;
        is_flapping  = false;
        stamina      = config::K_FLAP_STAMINA;
        coyote_timer = config::K_COYOTE;
        anim         = PlayerAnim::Idle;
        anim_timer   = 0.0f;
        anim_frame   = 0;
        perk_timer   = 0.0f;
    }
};

} // namespace schreihuhn::game
