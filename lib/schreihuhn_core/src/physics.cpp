#include "schreihuhn/game/physics.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/core/math_types.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace schreihuhn::game {

namespace {

struct ColliderBox {
    double left;
    double right;
    float  top;
    float  bottom;
};

template <typename Fn>
void forEachSolidCollider(const world::WorldState& world, Fn&& fn) {
    for (std::size_t i = 0; i < world.islands.size(); ++i) {
        const auto& isl = world.islands[i];
        fn(ColliderBox{isl.x, isl.x + static_cast<double>(isl.w), isl.y, config::K_ISLAND_COLL_BOT});
    }
    for (std::size_t i = 0; i < world.bridges.size(); ++i) {
        const auto& seg = world.bridges[i];
        if (seg.intact) {
            fn(ColliderBox{seg.x, seg.x + static_cast<double>(config::K_BRIDGE_SEG_W),
                           seg.y, seg.y + config::K_BRIDGE_SEG_H});
        }
    }
    for (std::size_t i = 0; i < world.obstacles.size(); ++i) {
        const auto& obs = world.obstacles[i];
        if (!obs.falling) {
            fn(ColliderBox{obs.x, obs.x + static_cast<double>(obs.w), obs.y, obs.y + obs.h});
        }
    }
}

} // namespace

PhysicsStepResult Physics::step(PlayerState& player,
                                world::WorldState& world,
                                const audio::MicClassification& voice,
                                float dt) {
    PhysicsStepResult res{};

    // 1. Apply input to velocity (FR-GP-01..15 & FR-PK-04)
    const bool perk_active = (player.perk_timer > 0.0f);
    bool jumped = false;

    if (perk_active) {
        const bool voice_up = (voice.state == audio::VoiceState::Talk ||
                               voice.state == audio::VoiceState::Shout);
        if ((player.on_ground || player.coyote_timer > 0.0f) && voice_up) {
            player.vy           = -config::K_PERK_JUMP_VY;
            player.vx           = config::K_PERK_FLY_VX;
            player.on_ground    = false;
            player.coyote_timer = 0.0f;
            player.is_flapping  = true;
            jumped              = true;
            res.jumped_this_tick = true;
        } else if (player.on_ground) {
            player.vx = core::moveToward(player.vx, 0.0f, config::K_GROUND_DECEL * dt);
            player.is_flapping = false;
        }

        if (!player.on_ground) {
            player.stamina = config::K_FLAP_STAMINA;
            if (voice_up) {
                const float climb = (voice.state == audio::VoiceState::Shout)
                                        ? config::K_PERK_CLIMB_VY
                                        : (config::K_PERK_FLOAT_VY +
                                           voice.intensity * (config::K_PERK_CLIMB_VY - config::K_PERK_FLOAT_VY));
                player.vy = core::moveToward(player.vy, climb, 540.0f * dt);
                player.vx = core::moveToward(player.vx, config::K_PERK_FLY_VX, 450.0f * dt);
                player.is_flapping = true;
            } else {
                player.vy = std::min(player.vy + config::K_PERK_GRAVITY * dt, config::K_PERK_MAX_FALL);
                player.vx = core::moveToward(player.vx, config::K_AIR_VX, 140.0f * dt);
                player.is_flapping = false;
            }
        }
    } else {
        if ((player.on_ground || player.coyote_timer > 0.0f) &&
            voice.state == audio::VoiceState::Shout) {
            player.vy           = -config::K_JUMP_VY;
            player.vx           = config::K_AIR_VX;
            player.on_ground    = false;
            player.coyote_timer = 0.0f;
            player.is_flapping  = false;
            jumped              = true;
            res.jumped_this_tick = true;
        } else if (player.on_ground) {
            if (voice.state == audio::VoiceState::Still) {
                player.vx = core::moveToward(player.vx, 0.0f, config::K_GROUND_DECEL * dt);
            } else if (voice.state == audio::VoiceState::Talk) {
                const float target = config::K_WALK_MIN +
                                     voice.intensity * (config::K_WALK_MAX - config::K_WALK_MIN);
                const float rate = (player.vx < target)
                                       ? config::K_GROUND_ACCEL
                                       : config::K_GROUND_DECEL;
                player.vx = core::moveToward(player.vx, target, rate * dt);
            }
            player.is_flapping = false;
        }

        if (!player.on_ground) {
            player.vy = std::min(player.vy + config::K_GRAVITY * dt, config::K_MAX_FALL);
            if (!jumped && voice.state == audio::VoiceState::Shout && player.stamina > 0.0f) {
                player.vx = config::K_FLAP_VX;
                player.is_flapping = true;
                if (player.vy >= 0.0f) {
                    player.stamina = std::max(0.0f, player.stamina - dt);
                    if (player.stamina > 0.0f) {
                        player.vy = std::min(player.vy, config::K_FLAP_MAX_FALL);
                    } else {
                        player.is_flapping = false;
                        player.vx = config::K_AIR_VX;
                    }
                }
            } else if (!jumped) {
                if (player.is_flapping) {
                    player.vx = config::K_AIR_VX;
                }
                player.is_flapping = false;
            }
        }
    }

    // 2. Horizontal movement & AABB collision (FR-GP-20, FR-GP-21)
    player.world_x += static_cast<double>(player.vx * dt);
    const float top    = player.y;
    const float bottom = player.y + config::K_HITBOX_H;

    forEachSolidCollider(world, [&](const ColliderBox& col) {
        const double left  = player.world_x;
        const double right = player.world_x + static_cast<double>(config::K_HITBOX_W);
        if (bottom > col.top + 0.01f && top < col.bottom - 0.01f) {
            if (right > col.left && left < col.right) {
                player.world_x = col.left - static_cast<double>(config::K_HITBOX_W);
                player.vx = 0.0f;
                res.hit_wall = true;
            }
        }
    });

    // 3. Vertical movement & Swept Top-Surface Anti-Tunneling (FR-GP-20, FR-GP-22, AT-09)
    const float prev_bottom = player.y + config::K_HITBOX_H;
    player.y += player.vy * dt;
    const float new_bottom = player.y + config::K_HITBOX_H;

    const double left  = player.world_x;
    const double right = player.world_x + static_cast<double>(config::K_HITBOX_W);

    bool landed = false;
    if (player.vy >= 0.0f) {
        float best_top = std::numeric_limits<float>::max();
        forEachSolidCollider(world, [&](const ColliderBox& col) {
            if (right > col.left && left < col.right) {
                if (prev_bottom <= col.top + 0.5f && new_bottom >= col.top) {
                    if (col.top < best_top) {
                        best_top = col.top;
                        landed = true;
                    }
                }
            }
        });
        if (landed) {
            if (!player.on_ground) {
                res.landed_this_tick = true;
            }
            player.y = best_top - config::K_HITBOX_H;
            player.vy = 0.0f;
        }
    } else {
        // Rising: check head collision (FR-GP-22)
        float best_bottom = -std::numeric_limits<float>::max();
        bool hit_head = false;
        forEachSolidCollider(world, [&](const ColliderBox& col) {
            if (right > col.left && left < col.right) {
                if ((player.y + config::K_HITBOX_H) > col.top + 0.01f && player.y < col.bottom) {
                    if (col.bottom > best_bottom) {
                        best_bottom = col.bottom;
                        hit_head = true;
                    }
                }
            }
        });
        if (hit_head) {
            player.y = best_bottom;
            player.vy = 0.0f;
        }
    }

    // 3b. Upper screen ceiling barrier during perk flight (FR-PK-05)
    if (perk_active && player.y < config::K_PERK_CEILING_Y) {
        player.y = config::K_PERK_CEILING_Y;
        if (player.vy < 0.0f) {
            player.vy = 0.0f;
        }
    }

    // 4. Ground state, stamina reset & coyote time (FR-GP-10, FR-GP-13)
    if (landed) {
        player.on_ground    = true;
        player.stamina      = config::K_FLAP_STAMINA;
        player.coyote_timer = config::K_COYOTE;
        player.is_flapping  = false;
    } else {
        player.on_ground    = false;
        player.coyote_timer = std::max(0.0f, player.coyote_timer - dt);
    }

    // 5. Bridge stability & collapse (FR-MP-11..15)
    const float player_foot_y = player.y + config::K_HITBOX_H;
    for (std::size_t i = 0; i < world.bridges.size(); ++i) {
        auto& seg = world.bridges[i];
        if (seg.intact) {
            if (player.on_ground &&
                std::fabs(seg.y - player_foot_y) < 0.5f &&
                right > seg.x &&
                left < seg.x + static_cast<double>(config::K_BRIDGE_SEG_W)) {
                seg.stability -= dt;
                if (seg.stability <= 0.0f) {
                    seg.stability = 0.0f;
                    seg.intact = false;
                    seg.fall_vy = 0.0f;
                    // Any obstacle standing on this bridge segment falls with it (spec/05 5.5)
                    for (std::size_t k = 0; k < world.obstacles.size(); ++k) {
                        auto& obs = world.obstacles[k];
                        if (!obs.falling &&
                            obs.x + static_cast<double>(obs.w) > seg.x &&
                            obs.x < seg.x + static_cast<double>(config::K_BRIDGE_SEG_W) &&
                            std::fabs((obs.y + obs.h) - seg.y) < 1.0f) {
                            obs.falling = true;
                            obs.fall_vy = 0.0f;
                        }
                    }
                }
            }
        } else {
            seg.fall_vy += config::K_GRAVITY * dt;
            seg.y += seg.fall_vy * dt;
        }
    }

    for (std::size_t k = 0; k < world.obstacles.size(); ++k) {
        auto& obs = world.obstacles[k];
        if (obs.falling) {
            obs.fall_vy += config::K_GRAVITY * dt;
            obs.y += obs.fall_vy * dt;
        }
    }

    // 6. Void death check (FR-GP-30)
    if (player.y > config::K_VOID_Y) {
        res.died_void = true;
    }

    updateAnimation(player, dt);
    return res;
}

void Physics::updateAnimation(PlayerState& player, float dt) {
    PlayerAnim target = PlayerAnim::Idle;
    if (!player.on_ground) {
        if (player.is_flapping) {
            target = PlayerAnim::Flap;
        } else if (player.vy < 0.0f) {
            target = PlayerAnim::Jump;
        } else {
            target = PlayerAnim::Fall;
        }
    } else if (player.vx > 2.0f) {
        target = PlayerAnim::Walk;
    } else {
        target = PlayerAnim::Idle;
    }

    if (target != player.anim) {
        player.anim = target;
        player.anim_timer = 0.0f;
        player.anim_frame = 0;
        return;
    }

    player.anim_timer += dt;
    switch (player.anim) {
        case PlayerAnim::Idle:
            if (player.anim_timer >= 0.5f) {
                player.anim_timer -= 0.5f;
                player.anim_frame = static_cast<uint8_t>((player.anim_frame + 1u) % 2u);
            }
            break;
        case PlayerAnim::Walk: {
            const float norm = core::clampf((player.vx - config::K_WALK_MIN) /
                                            (config::K_WALK_MAX - config::K_WALK_MIN),
                                            0.0f, 1.0f);
            const float frame_dur = 0.16f - norm * 0.08f; // 160 ms .. 80 ms (spec/08 8.4)
            if (player.anim_timer >= frame_dur) {
                player.anim_timer -= frame_dur;
                player.anim_frame = static_cast<uint8_t>((player.anim_frame + 1u) % 4u);
            }
            break;
        }
        case PlayerAnim::Flap:
            if (player.anim_timer >= 0.06f) { // 60 ms
                player.anim_timer -= 0.06f;
                player.anim_frame = static_cast<uint8_t>((player.anim_frame + 1u) % 2u);
            }
            break;
        case PlayerAnim::Jump:
        case PlayerAnim::Fall:
            player.anim_frame = 0;
            break;
    }
}

} // namespace schreihuhn::game
