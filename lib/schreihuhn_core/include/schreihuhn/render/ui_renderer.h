#pragma once

#include <cstdint>
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/game/run_session.h"
#include "schreihuhn/hal/canvas155.h"
#include "schreihuhn/input/input_mapper.h"
#include "schreihuhn/persistence/save_data.h"

namespace schreihuhn::render {

// Verified UI hitboxes inside Safe-Circle (77, 77) R=72 (docs/04, 4.5)
namespace UILayout {
inline constexpr core::RectI kMenuArrowLeft   {0, 38, 46, 56};
inline constexpr core::RectI kMenuArrowRight  {109, 38, 46, 56};
inline constexpr core::RectI kMenuActionBtn   {26, 103, 103, 24};
inline constexpr core::RectI kMenuSoundBtn    {46, 128, 63, 27};

inline constexpr core::RectI kBuyDialogBox    {24, 52, 107, 60};
inline constexpr core::RectI kBuyBtnNo        {28, 86, 44, 20};
inline constexpr core::RectI kBuyBtnYes       {83, 86, 44, 20};

inline constexpr core::RectI kHudPauseBtn     {18, 18, 38, 34};

inline constexpr core::RectI kPauseBtnMenu       {10, 76, 65, 42};
inline constexpr core::RectI kPauseBtnMenuVis    {22, 84, 51, 24};
inline constexpr core::RectI kPauseBtnResume     {80, 76, 65, 42};
inline constexpr core::RectI kPauseBtnResumeVis  {82, 84, 51, 24};

inline constexpr core::RectI kGameOverBtnMenu     {10, 94, 65, 40};
inline constexpr core::RectI kGameOverBtnMenuVis  {23, 101, 50, 22};
inline constexpr core::RectI kGameOverBtnRetry    {80, 94, 65, 40};
inline constexpr core::RectI kGameOverBtnRetryVis {82, 101, 50, 22};
} // namespace UILayout

class UIRenderer {
public:
    static void renderSplash(hal::Canvas155& canvas,
                             float clock_sec,
                             bool mic_error);

    static void renderMainMenu(hal::Canvas155& canvas,
                               const persistence::SaveData& save,
                               persistence::ChickenId carousel_chicken,
                               float clock_sec,
                               float carousel_slide_timer,
                               float error_shake_timer,
                               bool show_mic_error,
                               const input::InputFrameEvents& input_ev);

    static void renderBuyDialog(hal::Canvas155& canvas,
                                persistence::ChickenId chicken,
                                uint32_t price,
                                const input::InputFrameEvents& input_ev);

    static void renderCountdown(hal::Canvas155& canvas,
                                uint32_t remaining_ms,
                                bool is_initial_calibration,
                                bool show_go);

    static void renderHUD(hal::Canvas155& canvas,
                          const game::RunSession& session,
                          const audio::MicClassification& voice,
                          bool sim_mode,
                          const input::InputFrameEvents& input_ev);

    static void renderPause(hal::Canvas155& canvas,
                            uint32_t score_m,
                            const input::InputFrameEvents& input_ev);

    static void renderGameOver(hal::Canvas155& canvas,
                               uint32_t score_m,
                               uint32_t run_coins,
                               uint32_t total_coins,
                               bool is_new_highscore,
                               bool buttons_active,
                               float clock_sec,
                               const input::InputFrameEvents& input_ev);
};

} // namespace schreihuhn::render
