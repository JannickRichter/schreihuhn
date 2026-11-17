#include "schreihuhn/game/run_session.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/game/physics.h"

#include <algorithm>
#include <cmath>

namespace schreihuhn::game {

void RunSession::start(uint32_t seed, uint32_t existing_highscore) {
    phase_ = RunPhase::Playing;
    player_.reset();
    generator_.reset(seed, world_);
    generator_.ensureGeneratedUpTo(
        world_, static_cast<double>(config::K_LOGICAL_SIZE) + config::K_LOOKAHEAD_PX);

    camera_x_ = 0.0;
    max_world_x_ = config::K_START_X;
    score_m_ = 0;
    run_coins_ = 0;
    existing_highscore_ = existing_highscore;
    new_highscore_beaten_ = false;
    highscore_highlight_timer_ = 0.0f;
    accumulator_ = 0.0f;
    dying_timer_ = 0.0f;
    elapsed_sec_ = 0.0f;
    feather_timer_ = 0.0f;
    committed_ = false;

    current_biome_ = world::BiomeId::Greenland;
    prev_biome_ = world::BiomeId::Greenland;
    underlying_biome_ = world::BiomeId::Greenland;
    biome_transition_ = 1.0f;
    biome_transition_dur_ = config::K_BIOME_TRANSITION_S;
    biome_banner_timer_ = 0.0f;
    last_biome_marker_x_ = -1.0;
    ambient_count_ = 0;
    fx_rng_.reseed(seed == 0u ? 1u : seed ^ 0x5A17u);
}

bool RunSession::tick(float frame_dt,
                      const audio::MicClassification& voice,
                      hal::IAudioOut* audio_out) {
    if (phase_ == RunPhase::Ended || phase_ == RunPhase::Idle) {
        return phase_ == RunPhase::Ended;
    }

    const float clamped_dt = std::min(std::max(0.0f, frame_dt), 0.1f);
    elapsed_sec_ += clamped_dt;
    if (highscore_highlight_timer_ > 0.0f) {
        highscore_highlight_timer_ = std::max(0.0f, highscore_highlight_timer_ - clamped_dt);
    }

    accumulator_ += clamped_dt;
    while (accumulator_ >= config::K_PHYS_DT) {
        accumulator_ -= config::K_PHYS_DT;
        stepFixed(config::K_PHYS_DT, voice, audio_out);
        if (phase_ == RunPhase::Ended) {
            break;
        }
    }

    return phase_ == RunPhase::Ended;
}

void RunSession::stepFixed(float dt,
                           const audio::MicClassification& voice,
                           hal::IAudioOut* audio_out) {
    if (phase_ == RunPhase::Dying) {
        // FR-GP-31: ignore input during fall animation, max K_DYING_MAX_SEC (<= 1 s)
        player_.perk_timer = 0.0f;
        audio::MicClassification silent{};
        Physics::step(player_, world_, silent, dt);
        updateParticles(dt);
        dying_timer_ += dt;
        if (dying_timer_ >= config::K_DYING_MAX_SEC || player_.y > config::K_VOID_Y + 32.0f) {
            phase_ = RunPhase::Ended;
        }
        return;
    }

    const PhysicsStepResult phys_res = Physics::step(player_, world_, voice, dt);

    // Tick active flight perk timer and restore underlying biome cleanly on expiry
    if (player_.perk_timer > 0.0f) {
        player_.perk_timer = std::max(0.0f, player_.perk_timer - dt);
        if (player_.perk_timer <= 0.0f) {
            prev_biome_ = world::BiomeId::Candy;
            current_biome_ = underlying_biome_;
            biome_transition_ = 0.0f;
            biome_transition_dur_ = config::K_BIOME_TRANSITION_S;
            biome_banner_timer_ = config::K_BIOME_BANNER_S;
            populateAmbientFor(underlying_biome_);
        }
    }

    if (phys_res.landed_this_tick) {
        spawnDust(player_.world_x + 3.0, player_.y + config::K_HITBOX_H);
    }

    if (player_.is_flapping) {
        feather_timer_ += dt;
        if (feather_timer_ >= 0.12f) {
            feather_timer_ -= 0.12f;
            spawnFeather(player_.world_x + 2.0, player_.y + 6.0f);
        }
    } else {
        feather_timer_ = 0.0f;
    }

    // Coin pickup (FR-MP-25, FR-GP-50)
    const double p_left   = player_.world_x;
    const double p_right  = player_.world_x + static_cast<double>(config::K_HITBOX_W);
    const float  p_top    = player_.y;
    const float  p_bottom = player_.y + config::K_HITBOX_H;

    for (std::size_t i = 0; i < world_.coins.size(); ++i) {
        auto& coin = world_.coins[i];
        if (coin.collected) {
            continue;
        }
        const double c_left   = coin.x + 1.0;
        const double c_right  = coin.x + 7.0;
        const float  c_top    = coin.y + 1.0f;
        const float  c_bottom = coin.y + 7.0f;

        if (p_right > c_left && p_left < c_right && p_bottom > c_top && p_top < c_bottom) {
            coin.collected = true;
            ++run_coins_;
            spawnCoinSparkle(coin.x, coin.y);
            if (audio_out != nullptr) {
                audio_out->playSound(audio::SoundId::Coin);
            }
        }
    }

    // White-packet Perk pickup (FR-PK-01..06)
    for (std::size_t i = 0; i < world_.perks.size(); ++i) {
        auto& pk = world_.perks[i];
        if (pk.collected) {
            continue;
        }
        if (p_right > pk.x && p_left < pk.x + 8.0 &&
            p_bottom > pk.y && p_top < pk.y + 8.0f) {
            pk.collected = true;
            player_.perk_timer = config::K_PERK_DURATION_S;
            prev_biome_ = current_biome_;
            current_biome_ = world::BiomeId::Candy;
            biome_transition_ = 0.0f;
            biome_transition_dur_ = config::K_PERK_TRANSITION_S;
            biome_banner_timer_ = config::K_BIOME_BANNER_S;
            populateAmbientFor(world::BiomeId::Candy);
            spawnCoinSparkle(pk.x, pk.y);
            if (audio_out != nullptr) {
                audio_out->playSound(audio::SoundId::Perk);
            }
        }
    }

    updateParticles(dt);
    syncBiomeTransition(dt);
    updateAmbientParticles(dt);

    // Camera & Score (FR-GP-35, FR-GP-40, FR-GP-41)
    if (player_.world_x > max_world_x_) {
        max_world_x_ = player_.world_x;
    }
    const double target_cam = player_.world_x - static_cast<double>(config::K_CAMERA_PLAYER_X);
    if (target_cam > camera_x_) {
        camera_x_ = target_cam;
    }

    const uint32_t new_score = static_cast<uint32_t>(
        std::floor((max_world_x_ - config::K_START_X) / 8.0));
    if (new_score > score_m_) {
        score_m_ = std::min(new_score, config::K_COINS_MAX);
        if (!new_highscore_beaten_ && existing_highscore_ > 0 && score_m_ > existing_highscore_) {
            new_highscore_beaten_ = true;
            highscore_highlight_timer_ = config::K_SCORE_HIGHLIGHT_S;
        } else if (!new_highscore_beaten_ && existing_highscore_ == 0 && score_m_ > 0) {
            // Beat 0 m highscore at end of run, keep flag without flashing at 1 m if undesirable;
            // still mark new_highscore_beaten_ for Game Over screen (UC-09a).
            new_highscore_beaten_ = true;
        }
    }

    // Stream world ahead and prune behind (FR-MP-31)
    generator_.ensureGeneratedUpTo(
        world_, camera_x_ + static_cast<double>(config::K_LOGICAL_SIZE) + config::K_LOOKAHEAD_PX);
    generator_.pruneBefore(world_, camera_x_ - config::K_PRUNE_BEHIND_PX);

    if (phys_res.died_void) {
        phase_ = RunPhase::Dying;
        dying_timer_ = 0.0f;
    }
}

bool RunSession::commitToSave(persistence::SaveData& save, hal::IStorage& storage) {
    if (committed_) {
        return true;
    }
    committed_ = true;

    const uint64_t total_coins = static_cast<uint64_t>(save.coins) + static_cast<uint64_t>(run_coins_);
    save.coins = static_cast<uint32_t>(
        std::min<uint64_t>(total_coins, static_cast<uint64_t>(config::K_COINS_MAX)));

    const uint32_t clamped_score = std::min(score_m_, config::K_COINS_MAX);
    if (clamped_score > save.highscore) {
        save.highscore = clamped_score;
        new_highscore_beaten_ = true;
    } else {
        new_highscore_beaten_ = false;
    }

    save.sanitize();
    return storage.save(save);
}

void RunSession::spawnCoinSparkle(double x, float y) {
    world_.particles.pushBack(world::Particle{x, y, 0.0f, -8.0f, 0.0f, 0.18f, 0});
}

void RunSession::spawnFeather(double x, float y) {
    world_.particles.pushBack(world::Particle{x, y, -12.0f, 15.0f, 0.0f, 0.35f, 1});
}

void RunSession::spawnDust(double x, float y) {
    world_.particles.pushBack(world::Particle{x - 2.0, y - 3.0f, -8.0f, -6.0f, 0.0f, 0.20f, 2});
    world_.particles.pushBack(world::Particle{x + 5.0, y - 3.0f, 8.0f, -6.0f, 0.0f, 0.20f, 2});
}

void RunSession::updateParticles(float dt) {
    for (std::size_t i = 0; i < world_.particles.size(); ++i) {
        auto& p = world_.particles[i];
        p.age += dt;
        p.x += static_cast<double>(p.vx * dt);
        p.y += p.vy * dt;
    }
    while (!world_.particles.empty() && world_.particles[0].age >= world_.particles[0].lifetime) {
        world_.particles.popFront();
    }
}

void RunSession::syncBiomeTransition(float dt) {
    if (biome_transition_ < 1.0f) {
        const float dur = std::max(0.05f, biome_transition_dur_);
        biome_transition_ = std::min(1.0f, biome_transition_ + dt / dur);
    }
    if (biome_banner_timer_ > 0.0f) {
        biome_banner_timer_ = std::max(0.0f, biome_banner_timer_ - dt);
    }

    for (std::size_t i = 0; i < world_.biome_markers.size(); ++i) {
        const auto& m = world_.biome_markers[i];
        if (m.world_x > last_biome_marker_x_ && player_.world_x + 40.0 >= m.world_x) {
            last_biome_marker_x_ = m.world_x;
            underlying_biome_ = m.biome;
            if (player_.perk_timer <= 0.0f && m.biome != current_biome_) {
                prev_biome_ = current_biome_;
                current_biome_ = m.biome;
                biome_transition_ = 0.0f;
                biome_transition_dur_ = config::K_BIOME_TRANSITION_S;
                biome_banner_timer_ = config::K_BIOME_BANNER_S;
                populateAmbientFor(current_biome_);
            }
        }
    }
}

void RunSession::populateAmbientFor(world::BiomeId b) {
    ambient_count_ = 0;
    if (b == world::BiomeId::Snow) {
        const uint8_t n = std::min<uint8_t>(config::K_AMBIENT_SNOW_COUNT,
                                            static_cast<uint8_t>(ambient_particles_.size()));
        for (uint8_t i = 0; i < n; ++i) {
            world::AmbientParticle p{};
            p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.y = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.vx = -static_cast<float>(fx_rng_.rangeInt(7, 16));
            p.vy = static_cast<float>(fx_rng_.rangeInt(20, 38));
            p.phase = static_cast<float>(fx_rng_.rangeInt(0, 100)) * 0.06f;
            p.kind = 0;
            p.biome = world::BiomeId::Snow;
            ambient_particles_[ambient_count_++] = p;
        }
    } else if (b == world::BiomeId::Lava) {
        const uint8_t nc = config::K_AMBIENT_LAVA_COMETS;
        for (uint8_t i = 0; i < nc && ambient_count_ < ambient_particles_.size(); ++i) {
            world::AmbientParticle p{};
            p.x = static_cast<float>(fx_rng_.rangeInt(25, config::K_LOGICAL_SIZE + 25));
            p.y = static_cast<float>(fx_rng_.rangeInt(-30, 80));
            p.vx = -static_cast<float>(fx_rng_.rangeInt(36, 56));
            p.vy = static_cast<float>(fx_rng_.rangeInt(50, 78));
            p.phase = static_cast<float>(i);
            p.kind = 1;
            p.biome = world::BiomeId::Lava;
            ambient_particles_[ambient_count_++] = p;
        }
        const uint8_t ne = config::K_AMBIENT_LAVA_EMBERS;
        for (uint8_t i = 0; i < ne && ambient_count_ < ambient_particles_.size(); ++i) {
            world::AmbientParticle p{};
            p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.y = static_cast<float>(fx_rng_.rangeInt(110, config::K_LOGICAL_SIZE - 1));
            p.vx = -static_cast<float>(fx_rng_.rangeInt(4, 12));
            p.vy = -static_cast<float>(fx_rng_.rangeInt(14, 26));
            p.phase = static_cast<float>(fx_rng_.rangeInt(0, 62)) * 0.1f;
            p.kind = 2;
            p.biome = world::BiomeId::Lava;
            ambient_particles_[ambient_count_++] = p;
        }
    } else if (b == world::BiomeId::Jungle) {
        const uint8_t n = std::min<uint8_t>(config::K_AMBIENT_RAIN_COUNT,
                                            static_cast<uint8_t>(ambient_particles_.size()));
        for (uint8_t i = 0; i < n; ++i) {
            world::AmbientParticle p{};
            p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.y = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.vx = -static_cast<float>(fx_rng_.rangeInt(8, 14));
            p.vy = static_cast<float>(fx_rng_.rangeInt(95, 125));
            p.phase = static_cast<float>(fx_rng_.rangeInt(0, 100)) * 0.1f;
            p.kind = 3;
            p.biome = world::BiomeId::Jungle;
            ambient_particles_[ambient_count_++] = p;
        }
    } else if (b == world::BiomeId::Candy) {
        const uint8_t n = std::min<uint8_t>(config::K_AMBIENT_CANDY_COUNT,
                                            static_cast<uint8_t>(ambient_particles_.size()));
        for (uint8_t i = 0; i < n; ++i) {
            world::AmbientParticle p{};
            p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            p.y = static_cast<float>(fx_rng_.rangeInt(16, config::K_LOGICAL_SIZE - 10));
            p.vx = -static_cast<float>(fx_rng_.rangeInt(18, 36));
            p.vy = static_cast<float>(fx_rng_.rangeInt(-10, 10));
            p.phase = static_cast<float>(fx_rng_.rangeInt(0, 62)) * 0.1f;
            p.kind = 4;
            p.biome = world::BiomeId::Candy;
            ambient_particles_[ambient_count_++] = p;
        }
    }
}

void RunSession::updateAmbientParticles(float dt) {
    if (ambient_count_ == 0) {
        return;
    }
    const bool keep_running = (current_biome_ == world::BiomeId::Snow ||
                               current_biome_ == world::BiomeId::Lava ||
                               current_biome_ == world::BiomeId::Jungle ||
                               current_biome_ == world::BiomeId::Candy);
    if (!keep_running && biome_transition_ >= 1.0f) {
        ambient_count_ = 0;
        return;
    }
    const float w = static_cast<float>(config::K_LOGICAL_SIZE);
    const float cam_shift = player_.vx * 0.22f * dt;
    for (uint8_t i = 0; i < ambient_count_; ++i) {
        auto& p = ambient_particles_[i];
        p.phase += dt * 3.0f;
        p.x += p.vx * dt - cam_shift;
        p.y += p.vy * dt;
        if (p.kind == 0) {
            p.x += std::sin(p.phase) * 5.0f * dt;
            if (p.y >= w) {
                p.y -= w;
                p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            }
            if (p.x < 0.0f) p.x += w;
            if (p.x >= w) p.x -= w;
        } else if (p.kind == 1) {
            if (p.y > 142.0f || p.x < -12.0f) {
                p.x = static_cast<float>(fx_rng_.rangeInt(40, config::K_LOGICAL_SIZE + 30));
                p.y = static_cast<float>(fx_rng_.rangeInt(-24, 10));
            }
        } else if (p.kind == 2) {
            if (p.y < 100.0f || p.x < -4.0f) {
                p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
                p.y = static_cast<float>(fx_rng_.rangeInt(142, config::K_LOGICAL_SIZE - 1));
            }
        } else if (p.kind == 3) {
            if (p.y >= w) {
                p.y -= w;
                p.x = static_cast<float>(fx_rng_.rangeInt(0, config::K_LOGICAL_SIZE - 1));
            }
            if (p.x < 0.0f) p.x += w;
            if (p.x >= w) p.x -= w;
        } else {
            p.y += std::sin(p.phase * 2.0f) * 14.0f * dt;
            if (p.x < 0.0f) p.x += w;
            if (p.x >= w) p.x -= w;
            if (p.y < 14.0f) p.y = 140.0f;
            if (p.y > 148.0f) p.y = 20.0f;
        }
    }
}

} // namespace schreihuhn::game
