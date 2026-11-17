#pragma once

#include <array>
#include <cstdint>
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/game/player.h"
#include "schreihuhn/hal/i_audio_out.h"
#include "schreihuhn/hal/i_storage.h"
#include "schreihuhn/persistence/save_data.h"
#include "schreihuhn/world/map_generator.h"
#include "schreihuhn/world/world_types.h"

namespace schreihuhn::game {

enum class RunPhase : uint8_t {
    Idle    = 0,
    Playing = 1,
    Dying   = 2,
    Ended   = 3
};

class RunSession {
public:
    void start(uint32_t seed, uint32_t existing_highscore);

    // Fixed-timestep accumulator update; returns true when phase reaches Ended.
    bool tick(float frame_dt,
              const audio::MicClassification& voice,
              hal::IAudioOut* audio_out);

    // Commits run_coins and score_m to save and persists via storage (UC-09a).
    // Safe to call once per run; subsequent calls are no-ops.
    bool commitToSave(persistence::SaveData& save, hal::IStorage& storage);

    RunPhase phase() const { return phase_; }
    const PlayerState& player() const { return player_; }
    const world::WorldState& world() const { return world_; }
    double cameraX() const { return camera_x_; }
    uint32_t scoreM() const { return score_m_; }
    uint32_t runCoins() const { return run_coins_; }
    bool isNewHighscore() const { return new_highscore_beaten_; }
    float highscoreHighlightTimer() const { return highscore_highlight_timer_; }
    float elapsedTime() const { return elapsed_sec_; }
    bool committed() const { return committed_; }

    world::BiomeId currentBiome() const { return current_biome_; }
    world::BiomeId prevBiome() const { return prev_biome_; }
    float biomeTransitionProgress() const { return biome_transition_; }
    float biomeBannerTimer() const { return biome_banner_timer_; }
    const std::array<world::AmbientParticle, 24>& ambientParticles() const { return ambient_particles_; }
    uint8_t ambientCount() const { return ambient_count_; }

private:
    void stepFixed(float dt, const audio::MicClassification& voice, hal::IAudioOut* audio_out);
    void spawnCoinSparkle(double x, float y);
    void spawnFeather(double x, float y);
    void spawnDust(double x, float y);
    void updateParticles(float dt);
    void syncBiomeTransition(float dt);
    void populateAmbientFor(world::BiomeId b);
    void updateAmbientParticles(float dt);

    RunPhase phase_ = RunPhase::Idle;
    PlayerState player_{};
    world::WorldState world_{};
    world::MapGenerator generator_{};

    double camera_x_ = 0.0;
    double max_world_x_ = 0.0;
    uint32_t score_m_ = 0;
    uint32_t run_coins_ = 0;
    uint32_t existing_highscore_ = 0;
    bool new_highscore_beaten_ = false;
    float highscore_highlight_timer_ = 0.0f;

    float accumulator_ = 0.0f;
    float dying_timer_ = 0.0f;
    float elapsed_sec_ = 0.0f;
    float feather_timer_ = 0.0f;
    bool committed_ = false;

    world::BiomeId current_biome_ = world::BiomeId::Greenland;
    world::BiomeId prev_biome_ = world::BiomeId::Greenland;
    world::BiomeId underlying_biome_ = world::BiomeId::Greenland;
    float biome_transition_ = 1.0f;
    float biome_transition_dur_ = 1.4f;
    float biome_banner_timer_ = 0.0f;
    double last_biome_marker_x_ = -1.0;

    std::array<world::AmbientParticle, 24> ambient_particles_{};
    uint8_t ambient_count_ = 0;
    core::XorShift32 fx_rng_{1};
};

} // namespace schreihuhn::game
