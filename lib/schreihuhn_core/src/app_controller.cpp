#include "schreihuhn/app/app_controller.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/persistence/shop_logic.h"
#include "schreihuhn/render/ui_renderer.h"
#include "schreihuhn/render/world_renderer.h"

#include <algorithm>

namespace schreihuhn::app {

void AppController::init(hal::IStorage& storage,
                         hal::IAudioOut& audio,
                         hal::IHaptics& haptics,
                         audio::MicClassifier& mic,
                         bool mic_available,
                         uint32_t initial_seed) {
    storage_ = &storage;
    audio_   = &audio;
    haptics_ = &haptics;
    mic_     = &mic;
    mic_available_ = mic_available;
    rng_seed_state_ = (initial_seed == 0u) ? 0x13572468u : initial_seed;

    save_ = persistence::SaveData{};
    if (!storage_->load(save_)) {
        save_.sanitize();
        storage_->save(save_);
    } else {
        save_.sanitize();
    }

    audio_->setSoundEnabled(save_.sound_on);
    carousel_index_ = save_.selected;
    clock_sec_ = 0.0f;

    enterState(AppState::Splash, config::K_SCREEN_LOCKOUT_MS);
    state_timer_ms_ = config::K_SPLASH_MS;
}

void AppController::enterState(AppState next, uint16_t lockout_ms) {
    state_ = next;
    input_mapper_.onScreenChanged(lockout_ms);
}

uint32_t AppController::nextSeed() {
    uint32_t x = rng_seed_state_;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    if (x == 0u) x = 1u;
    rng_seed_state_ = x;
    return x;
}

void AppController::startNewRun() {
    if (save_.selected != carousel_index_) {
        save_.selected = carousel_index_;
        save_.sanitize();
        storage_->save(save_);
    }

    session_.start(nextSeed(), save_.highscore);
    mic_->reset();

    // UC-05 step 4: play start jingle before calibration so it does not pollute noise floor
    if (audio_->isSoundEnabled()) {
        audio_->playSound(audio::SoundId::Start);
        const auto seq = audio::getSoundSequence(audio::SoundId::Start);
        pre_calib_wait_ms_ = static_cast<uint32_t>(seq.total_duration_ms) +
                             static_cast<uint32_t>(config::K_GATE_TAIL_MS);
    } else {
        pre_calib_wait_ms_ = 0;
        mic_->beginCalibration();
    }

    state_timer_ms_ = config::K_CALIB_MS;
    go_banner_ms_ = 0;
    enterState(AppState::Countdown, config::K_SCREEN_LOCKOUT_MS);
}

bool AppController::wantsMicActive() const {
    if (state_ == AppState::Countdown) {
        return pre_calib_wait_ms_ == 0;
    }
    if (state_ == AppState::Playing) {
        return session_.phase() == game::RunPhase::Playing;
    }
    return false;
}

void AppController::tick(uint32_t dt_ms, const hal::PlatformInput& raw_input) {
    const float dt_sec = static_cast<float>(dt_ms) * 0.001f;
    clock_sec_ += dt_sec;

    if (carousel_slide_timer_ > 0.0f) {
        carousel_slide_timer_ = std::max(0.0f, carousel_slide_timer_ - dt_sec);
    }
    if (error_shake_timer_ > 0.0f) {
        error_shake_timer_ = std::max(0.0f, error_shake_timer_ - dt_sec);
    }
    if (mic_error_banner_timer_ > 0.0f) {
        mic_error_banner_timer_ = std::max(0.0f, mic_error_banner_timer_ - dt_sec);
    }

    sim_mode_active_ = raw_input.sim_talk || raw_input.sim_shout;

    // In Playing state, SIM_INPUT suppresses button events from InputMapper
    hal::PlatformInput mapped_in = raw_input;
    if (state_ == AppState::Playing && sim_mode_active_) {
        mapped_in.btn_left_down  = false;
        mapped_in.btn_right_down = false;
    }

    last_input_ev_ = input_mapper_.update(mapped_in, dt_ms);

    switch (state_) {
        case AppState::Splash: {
            if (state_timer_ms_ <= dt_ms) {
                state_timer_ms_ = 0;
                enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
            } else {
                state_timer_ms_ -= dt_ms;
                if (last_input_ev_.anyKeyOrTap()) {
                    enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
                }
            }
            break;
        }

        case AppState::MainMenu: {
            const auto& ev = last_input_ev_;
            const auto chicken = carouselChicken();

            if (ev.left_short || ev.tapInRect(render::UILayout::kMenuArrowLeft)) {
                carousel_index_ = static_cast<uint8_t>((carousel_index_ + 2u) % 3u);
                carousel_slide_timer_ = 0.15f;
                audio_->playSound(audio::SoundId::Click);
            } else if (ev.right_short || ev.tapInRect(render::UILayout::kMenuArrowRight)) {
                carousel_index_ = static_cast<uint8_t>((carousel_index_ + 1u) % 3u);
                carousel_slide_timer_ = 0.15f;
                audio_->playSound(audio::SoundId::Click);
            } else if (ev.left_long || ev.tapInRect(render::UILayout::kMenuSoundBtn)) {
                save_.sound_on = !save_.sound_on;
                audio_->setSoundEnabled(save_.sound_on);
                storage_->save(save_);
                if (save_.sound_on) {
                    audio_->playSound(audio::SoundId::Toggle);
                }
            } else if (ev.right_long || ev.tapInRect(render::UILayout::kMenuActionBtn)) {
                if (save_.isUnlocked(chicken)) {
                    if (!mic_available_ && !sim_mode_active_) {
                        mic_error_banner_timer_ = 2.0f;
                        audio_->playSound(audio::SoundId::Error);
                    } else {
                        startNewRun();
                    }
                } else if (persistence::ShopLogic::canAfford(save_, chicken)) {
                    audio_->playSound(audio::SoundId::Click);
                    enterState(AppState::BuyDialog, config::K_SCREEN_LOCKOUT_MS);
                } else {
                    error_shake_timer_ = 0.35f;
                    audio_->playSound(audio::SoundId::Error);
                }
            }
            break;
        }

        case AppState::BuyDialog: {
            const auto& ev = last_input_ev_;
            const auto chicken = carouselChicken();

            if (ev.right_short || ev.tapInRect(render::UILayout::kBuyBtnYes)) {
                if (persistence::ShopLogic::buy(save_, chicken, *storage_)) {
                    audio_->playSound(audio::SoundId::Buy);
                    haptics_->vibrate(config::K_HAPTIC_BUY_MS);
                }
                enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
            } else if (ev.left_short || ev.tapInRect(render::UILayout::kBuyBtnNo) ||
                       (ev.tap_valid &&
                        (!render::UILayout::kBuyDialogBox.contains(ev.tap_press_x, ev.tap_press_y) ||
                         !render::UILayout::kBuyDialogBox.contains(ev.tap_release_x, ev.tap_release_y)))) {
                audio_->playSound(audio::SoundId::Click);
                enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
            }
            break;
        }

        case AppState::Countdown: {
            if (pre_calib_wait_ms_ > 0) {
                if (dt_ms >= pre_calib_wait_ms_) {
                    pre_calib_wait_ms_ = 0;
                    mic_->beginCalibration();
                } else {
                    pre_calib_wait_ms_ -= dt_ms;
                }
                break;
            }
            if (dt_ms >= state_timer_ms_) {
                state_timer_ms_ = 0;
                mic_->finishCalibration();
                go_banner_ms_ = config::K_GO_BANNER_MS;
                enterState(AppState::Playing, config::K_SCREEN_LOCKOUT_MS);
            } else {
                state_timer_ms_ -= dt_ms;
            }
            break;
        }

        case AppState::Playing: {
            if (go_banner_ms_ > 0) {
                go_banner_ms_ = (dt_ms >= go_banner_ms_) ? 0 : (go_banner_ms_ - dt_ms);
            }

            if (session_.phase() == game::RunPhase::Playing) {
                const bool pause_req = last_input_ev_.tapInRect(render::UILayout::kHudPauseBtn) ||
                                       (!sim_mode_active_ && last_input_ev_.left_short);
                if (pause_req) {
                    audio_->playSound(audio::SoundId::Click);
                    enterState(AppState::Paused, config::K_SCREEN_LOCKOUT_MS);
                    break;
                }
            }

            audio::MicClassification voice = mic_->current();
            if (sim_mode_active_) {
                if (raw_input.sim_shout) {
                    voice.state = audio::VoiceState::Shout;
                    voice.intensity = 0.0f;
                } else if (raw_input.sim_talk) {
                    voice.state = audio::VoiceState::Talk;
                    voice.intensity = 0.5f;
                }
            }

            const bool ended = session_.tick(dt_sec, voice, audio_);
            if (ended) {
                // UC-09a: persist BEFORE showing Game Over
                session_.commitToSave(save_, *storage_);
                audio_->playSound(audio::SoundId::GameOver);
                haptics_->vibrate(config::K_HAPTIC_DEATH_MS);
                if (session_.isNewHighscore()) {
                    const auto seq = audio::getSoundSequence(audio::SoundId::GameOver);
                    hiscore_sfx_delay_ms_ = static_cast<uint32_t>(seq.total_duration_ms) + 40u;
                    hiscore_sfx_pending_ = true;
                } else {
                    hiscore_sfx_pending_ = false;
                }
                enterState(AppState::GameOver, config::K_GAMEOVER_LOCK_MS);
            }
            break;
        }

        case AppState::Paused: {
            const auto& ev = last_input_ev_;
            if (ev.left_short || ev.tapInRect(render::UILayout::kPauseBtnMenu)) {
                session_.commitToSave(save_, *storage_);
                audio_->playSound(audio::SoundId::Click);
                carousel_index_ = save_.selected;
                enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
            } else if (ev.right_short || ev.tapInRect(render::UILayout::kPauseBtnResume)) {
                audio_->playSound(audio::SoundId::Click);
                state_timer_ms_ = config::K_RESUME_MS;
                enterState(AppState::ResumeCountdown, config::K_SCREEN_LOCKOUT_MS);
            }
            break;
        }

        case AppState::ResumeCountdown: {
            if (dt_ms >= state_timer_ms_) {
                state_timer_ms_ = 0;
                go_banner_ms_ = 0;
                enterState(AppState::Playing, config::K_SCREEN_LOCKOUT_MS);
            } else {
                state_timer_ms_ -= dt_ms;
            }
            break;
        }

        case AppState::GameOver: {
            if (hiscore_sfx_pending_) {
                if (dt_ms >= hiscore_sfx_delay_ms_) {
                    hiscore_sfx_delay_ms_ = 0;
                    hiscore_sfx_pending_ = false;
                    audio_->playSound(audio::SoundId::Hiscore);
                } else {
                    hiscore_sfx_delay_ms_ -= dt_ms;
                }
            }
            const auto& ev = last_input_ev_;
            if (ev.left_short || ev.tapInRect(render::UILayout::kGameOverBtnMenu)) {
                audio_->playSound(audio::SoundId::Click);
                carousel_index_ = save_.selected;
                enterState(AppState::MainMenu, config::K_SCREEN_LOCKOUT_MS);
            } else if (ev.right_short || ev.tapInRect(render::UILayout::kGameOverBtnRetry)) {
                audio_->playSound(audio::SoundId::Click);
                startNewRun();
            }
            break;
        }
    }
}

void AppController::render(hal::Canvas155& canvas) const {
    const auto active_chicken = static_cast<persistence::ChickenId>(save_.selected);

    switch (state_) {
        case AppState::Splash:
            render::UIRenderer::renderSplash(canvas, clock_sec_, !mic_available_);
            break;

        case AppState::MainMenu:
            render::UIRenderer::renderMainMenu(
                canvas, save_, carouselChicken(), clock_sec_,
                carousel_slide_timer_, error_shake_timer_,
                mic_error_banner_timer_ > 0.0f, last_input_ev_);
            break;

        case AppState::BuyDialog:
            render::UIRenderer::renderMainMenu(
                canvas, save_, carouselChicken(), clock_sec_,
                0.0f, 0.0f, false, input::InputFrameEvents{});
            render::UIRenderer::renderBuyDialog(
                canvas, carouselChicken(),
                persistence::ShopLogic::priceOf(carouselChicken()), last_input_ev_);
            break;

        case AppState::Countdown:
            render::WorldRenderer::render(canvas, session_, active_chicken, clock_sec_);
            render::UIRenderer::renderCountdown(canvas, state_timer_ms_, true, false);
            break;

        case AppState::Playing:
            render::WorldRenderer::render(canvas, session_, active_chicken, clock_sec_);
            render::UIRenderer::renderHUD(canvas, session_, mic_->current(),
                                          sim_mode_active_, last_input_ev_);
            if (go_banner_ms_ > 0) {
                render::UIRenderer::renderCountdown(canvas, 0, false, true);
            }
            break;

        case AppState::Paused:
            render::WorldRenderer::render(canvas, session_, active_chicken, clock_sec_);
            render::UIRenderer::renderPause(canvas, session_.scoreM(), last_input_ev_);
            break;

        case AppState::ResumeCountdown:
            render::WorldRenderer::render(canvas, session_, active_chicken, clock_sec_);
            render::UIRenderer::renderCountdown(canvas, state_timer_ms_, false, false);
            break;

        case AppState::GameOver:
            render::UIRenderer::renderGameOver(
                canvas,
                session_.scoreM(),
                session_.runCoins(),
                save_.coins,
                session_.isNewHighscore(),
                !input_mapper_.isLockedOut(),
                clock_sec_,
                last_input_ev_);
            break;
    }
}

} // namespace schreihuhn::app
