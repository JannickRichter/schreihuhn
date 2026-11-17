#pragma once

#include <cstdint>
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/game/run_session.h"
#include "schreihuhn/hal/canvas155.h"
#include "schreihuhn/hal/i_audio_out.h"
#include "schreihuhn/hal/i_haptics.h"
#include "schreihuhn/hal/i_storage.h"
#include "schreihuhn/hal/platform_input.h"
#include "schreihuhn/input/input_mapper.h"
#include "schreihuhn/persistence/save_data.h"

namespace schreihuhn::app {

enum class AppState : uint8_t {
    Splash          = 0,
    MainMenu        = 1,
    BuyDialog       = 2,
    Countdown       = 3,
    Playing         = 4,
    Paused          = 5,
    ResumeCountdown = 6,
    GameOver        = 7
};

class AppController {
public:
    void init(hal::IStorage& storage,
              hal::IAudioOut& audio,
              hal::IHaptics& haptics,
              audio::MicClassifier& mic,
              bool mic_available,
              uint32_t initial_seed);

    void tick(uint32_t dt_ms, const hal::PlatformInput& raw_input);
    void render(hal::Canvas155& canvas) const;

    AppState state() const { return state_; }
    const persistence::SaveData& saveData() const { return save_; }
    persistence::ChickenId carouselChicken() const {
        return static_cast<persistence::ChickenId>(carousel_index_);
    }
    const game::RunSession& runSession() const { return session_; }
    bool wantsMicActive() const;
    void setMicAvailable(bool available) { mic_available_ = available; }

private:
    void enterState(AppState next, uint16_t lockout_ms);
    void startNewRun();
    uint32_t nextSeed();

    hal::IStorage*        storage_ = nullptr;
    hal::IAudioOut*       audio_   = nullptr;
    hal::IHaptics*        haptics_ = nullptr;
    audio::MicClassifier* mic_     = nullptr;

    AppState state_ = AppState::Splash;
    persistence::SaveData save_{};
    uint8_t carousel_index_ = 0;

    input::InputMapper input_mapper_{};
    input::InputFrameEvents last_input_ev_{};
    game::RunSession session_{};

    bool mic_available_ = true;
    bool sim_mode_active_ = false;
    uint32_t rng_seed_state_ = 1;

    float clock_sec_ = 0.0f;
    uint32_t state_timer_ms_ = 0;
    uint32_t pre_calib_wait_ms_ = 0;
    uint32_t go_banner_ms_ = 0;
    uint32_t hiscore_sfx_delay_ms_ = 0;
    bool hiscore_sfx_pending_ = false;

    float carousel_slide_timer_ = 0.0f;
    float error_shake_timer_ = 0.0f;
    float mic_error_banner_timer_ = 0.0f;
};

} // namespace schreihuhn::app
