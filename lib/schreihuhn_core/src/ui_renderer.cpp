#include "schreihuhn/render/ui_renderer.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/persistence/shop_logic.h"
#include "schreihuhn/render/font5x7.h"
#include "schreihuhn/render/palette.h"
#include "schreihuhn/render/sprites.h"
#include "schreihuhn/render/world_renderer.h"

#include <cmath>
#include <cstdio>
#include <cstring>

namespace schreihuhn::render {

namespace {

void formatWithDots(uint32_t val, char* out, std::size_t cap) {
    char raw[16];
    std::snprintf(raw, sizeof(raw), "%u", static_cast<unsigned>(val));
    const std::size_t len = std::strlen(raw);
    std::size_t out_idx = 0;
    for (std::size_t i = 0; i < len && out_idx + 1 < cap; ++i) {
        if (i > 0 && ((len - i) % 3u) == 0u) {
            out[out_idx++] = '.';
        }
        out[out_idx++] = raw[i];
    }
    out[out_idx] = '\0';
}

void drawButton(hal::Canvas155& canvas,
                const core::RectI& rect,
                const char* label,
                uint8_t fill_col,
                uint8_t text_col,
                bool pressed,
                bool disabled) {
    const uint8_t bg = disabled ? static_cast<uint8_t>(C_DARK_GREY)
                                : (pressed ? static_cast<uint8_t>(C_ORANGE) : fill_col);
    const uint8_t border = pressed ? C_WHITE : C_BLACK;
    const uint8_t tc = disabled ? static_cast<uint8_t>(C_LIGHT_GREY)
                                : (pressed ? static_cast<uint8_t>(C_BLACK) : text_col);
    const int dy = pressed ? 1 : 0;

    canvas.fillRect(rect.x, rect.y + dy, rect.w, rect.h, bg);
    canvas.drawRectOutline(rect.x, rect.y + dy, rect.w, rect.h, border);
    if (!pressed && !disabled) {
        canvas.fillRect(rect.x + 1, rect.y + 1, rect.w - 2, 1, C_WHITE);
    }

    const int tw = Font5x7::measureWidth(label, 1);
    const int tx = rect.x + (rect.w - tw) / 2;
    const int ty = rect.y + (rect.h - 7) / 2 + dy;
    if (disabled) {
        Font5x7::drawText(canvas, tx, ty, label, tc, 1);
    } else {
        Font5x7::drawTextOutlined(canvas, tx, ty, label, tc, C_BLACK, 1);
    }
}

} // namespace

void UIRenderer::renderSplash(hal::Canvas155& canvas, float clock_sec, bool mic_error) {
    canvas.clear(C_BLACK);

    // Minimal boot logo: small centered chicken icon on black AMOLED background
    const uint8_t idle_frame = static_cast<uint8_t>(static_cast<int>(clock_sec / 0.35f) % 2);
    Sprites::drawChicken(canvas, 61, 61, persistence::ChickenId::Henni,
                         game::PlayerAnim::Idle, idle_frame, false, 2);

    if (mic_error) {
        Font5x7::drawTextCentered(canvas, 77, 112, "MIKROFON-FEHLER", C_RED, C_BLACK, 1);
    }
}

void UIRenderer::renderMainMenu(hal::Canvas155& canvas,
                                const persistence::SaveData& save,
                                persistence::ChickenId carousel_chicken,
                                float clock_sec,
                                float carousel_slide_timer,
                                float error_shake_timer,
                                bool show_mic_error,
                                const input::InputFrameEvents& input_ev) {
    WorldRenderer::renderSkyAndParallax(canvas, static_cast<double>(clock_sec * 8.0f));

    // Coins (top centered)
    char num_buf[20];
    formatWithDots(save.coins, num_buf, sizeof(num_buf));
    const int nw = Font5x7::measureWidth(num_buf, 1);
    const int total_cw = 9 + nw;
    const int cx = 77 - total_cw / 2;
    Sprites::drawCoinSmall(canvas, cx, 13);
    Font5x7::drawTextOutlined(canvas, cx + 9, 13, num_buf, C_YELLOW, C_BLACK, 1);

    // Highscore
    char hi_buf[24];
    std::snprintf(hi_buf, sizeof(hi_buf), "HI %u m", static_cast<unsigned>(save.highscore));
    Font5x7::drawTextCentered(canvas, 77, 24, hi_buf, C_WHITE, C_BLACK, 1);

    // Carousel arrows with visible touch pads aligned to outer hitboxes
    const bool left_pressed = input_ev.left_visual_down || input_ev.touchHeldInRect(UILayout::kMenuArrowLeft);
    const bool right_pressed = input_ev.right_visual_down || input_ev.touchHeldInRect(UILayout::kMenuArrowRight);
    canvas.fillRect(10, 52, 18, 22, left_pressed ? C_ORANGE : C_DARK_BLUE);
    canvas.drawRectOutline(10, 52, 18, 22, left_pressed ? C_WHITE : C_BLACK);
    Sprites::drawArrowLeft(canvas, 15, 57, left_pressed ? C_BLACK : C_WHITE);

    canvas.fillRect(127, 52, 18, 22, right_pressed ? C_ORANGE : C_DARK_BLUE);
    canvas.drawRectOutline(127, 52, 18, 22, right_pressed ? C_WHITE : C_BLACK);
    Sprites::drawArrowRight(canvas, 132, 57, right_pressed ? C_BLACK : C_WHITE);

    // Chicken 3x preview (48x48)
    const bool unlocked = save.isUnlocked(carousel_chicken);
    const uint8_t idle_frame = static_cast<uint8_t>(static_cast<int>(clock_sec / 0.5f) % 2);
    int slide_dx = 0;
    if (carousel_slide_timer > 0.0f) {
        slide_dx = ((static_cast<int>(carousel_slide_timer * 60.0f) & 1) != 0) ? 2 : -2;
    }
    Sprites::drawChicken(canvas, 53 + slide_dx, 39, carousel_chicken,
                         game::PlayerAnim::Idle, idle_frame, !unlocked, 3);
    if (!unlocked) {
        Sprites::drawLock(canvas, 73, 58);
    }

    // Chicken name & status
    Font5x7::drawTextCentered(canvas, 77, 89,
                              persistence::ShopLogic::nameOf(carousel_chicken), C_WHITE, C_BLACK, 1);

    const uint32_t price = persistence::ShopLogic::priceOf(carousel_chicken);
    if (unlocked) {
        if (save.selected == static_cast<uint8_t>(carousel_chicken)) {
            Font5x7::drawTextCentered(canvas, 77, 98, "GEWÄHLT", C_GREEN, C_BLACK, 1);
        }
    } else {
        char price_buf[16];
        std::snprintf(price_buf, sizeof(price_buf), "%u COINS", static_cast<unsigned>(price));
        Font5x7::drawTextCentered(canvas, 77, 98, price_buf, C_YELLOW, C_BLACK, 1);
    }

    // Action button
    core::RectI btn_rect{31, 105, 93, 21};
    if (error_shake_timer > 0.0f) {
        btn_rect.x += ((static_cast<int>(error_shake_timer * 40.0f) & 1) != 0) ? 2 : -2;
    }
    const bool action_pressed = input_ev.touchHeldInRect(UILayout::kMenuActionBtn);

    if (unlocked) {
        drawButton(canvas, btn_rect, "START", C_GREEN, C_WHITE, action_pressed, false);
    } else if (persistence::ShopLogic::canAfford(save, carousel_chicken)) {
        char buy_buf[24];
        std::snprintf(buy_buf, sizeof(buy_buf), "KAUFEN %u", static_cast<unsigned>(price));
        drawButton(canvas, btn_rect, buy_buf, C_YELLOW, C_BLACK, action_pressed, false);
    } else {
        const uint32_t missing = persistence::ShopLogic::missingCoins(save, carousel_chicken);
        char miss_buf[24];
        std::snprintf(miss_buf, sizeof(miss_buf), "FEHLEN %u", static_cast<unsigned>(missing));
        drawButton(canvas, btn_rect, miss_buf, C_DARK_GREY, C_LIGHT_GREY, false, true);
    }

    // Sound toggle icon with centered pad aligned inside kMenuSoundBtn
    const bool snd_pressed = input_ev.touchHeldInRect(UILayout::kMenuSoundBtn);
    canvas.fillRect(64, 131, 27, 16, snd_pressed ? C_ORANGE : C_DARK_BLUE);
    canvas.drawRectOutline(64, 131, 27, 16, snd_pressed ? C_WHITE : C_BLACK);
    Sprites::drawSpeaker(canvas, 72, 134, save.sound_on);

    if (show_mic_error) {
        canvas.fillRect(20, 128, 115, 12, C_BLACK);
        Font5x7::drawTextCentered(canvas, 77, 131, "MIKROFON-FEHLER", C_RED, C_BLACK, 1);
    }
}

void UIRenderer::renderBuyDialog(hal::Canvas155& canvas,
                                 persistence::ChickenId chicken,
                                 uint32_t price,
                                 const input::InputFrameEvents& input_ev) {
    canvas.drawDitherOverlay(C_BLACK);

    const auto& box = UILayout::kBuyDialogBox;
    canvas.fillRect(box.x, box.y, box.w, box.h, C_DARK_BLUE);
    canvas.drawRectOutline(box.x, box.y, box.w, box.h, C_WHITE);

    Font5x7::drawTextCentered(canvas, 77, 58, persistence::ShopLogic::nameOf(chicken),
                              C_YELLOW, C_BLACK, 1);

    char q_buf[32];
    std::snprintf(q_buf, sizeof(q_buf), "FÜR %u KAUFEN?", static_cast<unsigned>(price));
    Font5x7::drawTextCentered(canvas, 77, 70, q_buf, C_WHITE, C_BLACK, 1);

    const bool no_pressed = input_ev.left_visual_down || input_ev.touchHeldInRect(UILayout::kBuyBtnNo);
    const bool yes_pressed = input_ev.right_visual_down || input_ev.touchHeldInRect(UILayout::kBuyBtnYes);

    drawButton(canvas, UILayout::kBuyBtnNo, "NEIN", C_RED, C_WHITE, no_pressed, false);
    drawButton(canvas, UILayout::kBuyBtnYes, "JA", C_GREEN, C_WHITE, yes_pressed, false);
}

void UIRenderer::renderCountdown(hal::Canvas155& canvas,
                                 uint32_t remaining_ms,
                                 bool is_initial_calibration,
                                 bool show_go) {
    canvas.drawDitherOverlay(C_DARK_BLUE);
    if (show_go) {
        Font5x7::drawTextCentered(canvas, 77, 60, "LOS!", C_GREEN, C_BLACK, 2);
        return;
    }

    const char* num = "1";
    if (is_initial_calibration) {
        if (remaining_ms > 1000u) {
            num = "3";
        } else if (remaining_ms > 500u) {
            num = "2";
        } else {
            num = "1";
        }
    }
    Font5x7::drawTextCentered(canvas, 77, 60, num, C_YELLOW, C_BLACK, 2);
    if (is_initial_calibration) {
        Font5x7::drawTextCentered(canvas, 77, 82, "PSST... LEISE!", C_WHITE, C_BLACK, 1);
    }
}

void UIRenderer::renderHUD(hal::Canvas155& canvas,
                           const game::RunSession& session,
                           const audio::MicClassification& voice,
                           bool sim_mode,
                           const input::InputFrameEvents& input_ev) {
    // Score (FR-UI-30, FR-GP-41)
    char score_buf[16];
    std::snprintf(score_buf, sizeof(score_buf), "%u", static_cast<unsigned>(session.scoreM()));
    const int sw = Font5x7::measureWidth(score_buf, 2);
    const int mw = Font5x7::measureWidth("m", 1);
    const int total_w = sw + 2 + mw;
    const int sx = 77 - total_w / 2;
    const uint8_t score_col = (session.highscoreHighlightTimer() > 0.0f) ? C_YELLOW : C_WHITE;
    Font5x7::drawTextOutlined(canvas, sx, 12, score_buf, score_col, C_BLACK, 2);
    Font5x7::drawTextOutlined(canvas, sx + sw + 2, 19, "m", score_col, C_BLACK, 1);

    // Pause button (FR-UI-33)
    const bool pause_pressed = input_ev.touchHeldInRect(UILayout::kHudPauseBtn);
    Sprites::drawPauseIcon(canvas, 30, 28, pause_pressed ? C_YELLOW : C_WHITE);

    // Run-Coins (FR-UI-32)
    char coin_buf[16];
    std::snprintf(coin_buf, sizeof(coin_buf), "%u", static_cast<unsigned>(session.runCoins()));
    Sprites::drawCoinSmall(canvas, 98, 29);
    Font5x7::drawTextOutlined(canvas, 107, 29, coin_buf, C_YELLOW, C_BLACK, 1);

    // Mic level meter (FR-UI-31): x=52, y=138, w=50, h=5
    constexpr int mx = 52;
    constexpr int my = 138;
    constexpr int mw_bar = 50;
    constexpr int mh_bar = 5;

    canvas.fillRect(mx - 1, my - 1, mw_bar + 2, mh_bar + 2, C_BLACK);
    canvas.fillRect(mx, my, mw_bar, mh_bar, C_DARK_BLUE);

    // Map [-75 dB .. -15 dB] -> [0 .. 50]
    constexpr float db_min = -75.0f;
    constexpr float db_max = -15.0f;
    auto dbToPx = [&](float db) -> int {
        const float norm = core::clampf((db - db_min) / (db_max - db_min), 0.0f, 1.0f);
        return static_cast<int>(std::lround(norm * static_cast<float>(mw_bar)));
    };

    const int talk_px  = dbToPx(voice.talk_db);
    const int shout_px = dbToPx(voice.shout_db);
    int level_px = dbToPx(voice.smoothed_db);
    if (sim_mode) {
        if (voice.state == audio::VoiceState::Shout) level_px = mw_bar;
        else if (voice.state == audio::VoiceState::Talk) level_px = (talk_px + shout_px) / 2;
        else level_px = 0;
    }

    if (level_px > 0) {
        const int grey_w = std::min(level_px, talk_px);
        if (grey_w > 0) {
            canvas.fillRect(mx, my, grey_w, mh_bar, C_LIGHT_GREY);
        }
        if (level_px > talk_px) {
            const int green_w = std::min(level_px, shout_px) - talk_px;
            if (green_w > 0) {
                canvas.fillRect(mx + talk_px, my, green_w, mh_bar, C_GREEN);
            }
        }
        if (level_px > shout_px) {
            const int red_w = level_px - shout_px;
            if (red_w > 0) {
                canvas.fillRect(mx + shout_px, my, red_w, mh_bar, C_RED);
            }
        }
    }

    // Threshold markers
    canvas.fillRect(mx + std::min(mw_bar - 1, talk_px), my - 1, 1, mh_bar + 2, C_WHITE);
    canvas.fillRect(mx + std::min(mw_bar - 1, shout_px), my - 1, 1, mh_bar + 2, C_WHITE);

    // Perk flight timer bar (FR-PK-06)
    const float perk_t = session.player().perk_timer;
    if (perk_t > 0.0f) {
        constexpr int px = 55;
        constexpr int py = 24;
        constexpr int pw = 44;
        constexpr int ph = 4;
        const float ratio = std::clamp(perk_t / config::K_PERK_DURATION_S, 0.0f, 1.0f);
        const int fill = static_cast<int>(std::lround(ratio * static_cast<float>(pw - 2)));
        const bool warn = (perk_t <= config::K_PERK_WARN_S);
        const int pulse = static_cast<int>(session.elapsedTime() * 14.0f);
        uint8_t bar_col = ((pulse & 1) == 0) ? C_PINK : C_LAVENDER;
        if (warn) {
            bar_col = ((pulse & 1) == 0) ? C_WHITE : C_RED;
        }
        canvas.fillRect(px, py, pw, ph, C_WHITE);
        canvas.fillRect(px + 1, py + 1, pw - 2, ph - 2, C_BLACK);
        if (fill > 0) {
            canvas.fillRect(px + 1, py + 1, fill, ph - 2, bar_col);
        }
    }

    // Subtle biome transition banner (FR-BM-22, FR-PK-05)
    if (session.biomeBannerTimer() > 0.0f) {
        const char* bname = "GREENLAND";
        uint8_t bcol = C_GREEN;
        switch (session.currentBiome()) {
            case world::BiomeId::Taiga:
                bname = "TAIGA";
                bcol = C_LIGHT_GREY;
                break;
            case world::BiomeId::Snow:
                bname = "SCHNEEWELT";
                bcol = C_LIGHT_BLUE;
                break;
            case world::BiomeId::Lava:
                bname = "LAVAWELT";
                bcol = C_ORANGE;
                break;
            case world::BiomeId::Jungle:
                bname = "DSCHUNGEL";
                bcol = C_YELLOW;
                break;
            case world::BiomeId::Sand:
                bname = "WÜSTE";
                bcol = C_YELLOW;
                break;
            case world::BiomeId::Candy: {
                bname = "ZUCKERRAUSCH!";
                const int flash = static_cast<int>(session.elapsedTime() * 16.0f) % 3;
                bcol = (flash == 0) ? C_PINK : (flash == 1 ? C_WHITE : C_YELLOW);
                break;
            }
            default:
                break;
        }
        Font5x7::drawTextCentered(canvas, 77, 32, bname, bcol, C_BLACK, 1);
    }
}

void UIRenderer::renderPause(hal::Canvas155& canvas,
                             uint32_t score_m,
                             const input::InputFrameEvents& input_ev) {
    canvas.drawDitherOverlay(C_BLACK);

    Font5x7::drawTextCentered(canvas, 77, 44, "PAUSE", C_WHITE, C_BLACK, 2);

    char s_buf[24];
    std::snprintf(s_buf, sizeof(s_buf), "%u m", static_cast<unsigned>(score_m));
    Font5x7::drawTextCentered(canvas, 77, 64, s_buf, C_YELLOW, C_BLACK, 1);

    const bool menu_pressed = input_ev.left_visual_down || input_ev.touchHeldInRect(UILayout::kPauseBtnMenu);
    const bool resume_pressed = input_ev.right_visual_down || input_ev.touchHeldInRect(UILayout::kPauseBtnResume);

    drawButton(canvas, UILayout::kPauseBtnMenuVis, "MENÜ", C_RED, C_WHITE, menu_pressed, false);
    drawButton(canvas, UILayout::kPauseBtnResumeVis, "WEITER", C_GREEN, C_WHITE, resume_pressed, false);
}

void UIRenderer::renderGameOver(hal::Canvas155& canvas,
                                uint32_t score_m,
                                uint32_t run_coins,
                                uint32_t total_coins,
                                bool is_new_highscore,
                                bool buttons_active,
                                float clock_sec,
                                const input::InputFrameEvents& input_ev) {
    canvas.fillRect(0, 0, hal::Canvas155::kSize, hal::Canvas155::kSize, C_DARK_BLUE);

    Font5x7::drawTextCentered(canvas, 77, 26, "GAME OVER", C_RED, C_BLACK, 1);

    if (is_new_highscore) {
        const bool blink_on = (static_cast<int>(clock_sec * 4.0f) & 1) == 0;
        Font5x7::drawTextCentered(canvas, 77, 40, "NEUER HIGHSCORE!",
                                  blink_on ? C_YELLOW : C_ORANGE, C_BLACK, 1);
    }

    char score_buf[24];
    std::snprintf(score_buf, sizeof(score_buf), "%u m", static_cast<unsigned>(score_m));
    Font5x7::drawTextCentered(canvas, 77, 54, score_buf, C_WHITE, C_BLACK, 2);

    char rc_buf[24];
    std::snprintf(rc_buf, sizeof(rc_buf), "+ %u", static_cast<unsigned>(run_coins));
    const int rc_w = Font5x7::measureWidth(rc_buf, 1);
    const int rc_x = 77 - (9 + rc_w) / 2;
    Sprites::drawCoinSmall(canvas, rc_x, 74);
    Font5x7::drawTextOutlined(canvas, rc_x + 9, 74, rc_buf, C_YELLOW, C_BLACK, 1);

    char tot_buf[32];
    char dots_buf[20];
    formatWithDots(total_coins, dots_buf, sizeof(dots_buf));
    std::snprintf(tot_buf, sizeof(tot_buf), "GESAMT %s", dots_buf);
    Font5x7::drawTextCentered(canvas, 77, 86, tot_buf, C_LIGHT_GREY, C_BLACK, 1);

    const bool menu_pressed = buttons_active &&
                              (input_ev.left_visual_down || input_ev.touchHeldInRect(UILayout::kGameOverBtnMenu));
    const bool retry_pressed = buttons_active &&
                               (input_ev.right_visual_down || input_ev.touchHeldInRect(UILayout::kGameOverBtnRetry));

    drawButton(canvas, UILayout::kGameOverBtnMenuVis, "MENÜ", C_RED, C_WHITE,
               menu_pressed, !buttons_active);
    drawButton(canvas, UILayout::kGameOverBtnRetryVis, "NOCHMAL", C_GREEN, C_WHITE,
               retry_pressed, !buttons_active);
}

} // namespace schreihuhn::render
