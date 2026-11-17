#include <unity.h>

#include "mock_hal.h"
#include "schreihuhn/app/app_controller.h"
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/game/run_session.h"
#include "schreihuhn/hal/canvas155.h"
#include "schreihuhn/input/input_mapper.h"
#include "schreihuhn/persistence/save_data.h"
#include "schreihuhn/persistence/shop_logic.h"
#include "schreihuhn/render/palette.h"
#include "schreihuhn/render/ui_renderer.h"
#include "schreihuhn/render/world_renderer.h"

using namespace schreihuhn;

void setUp() {}
void tearDown() {}

// AT-16: Score == floor((x_max - 24) / 8), never decreases
void test_AT16_score_calculation_monotonic() {
    test::MockAudioOut audio_out;
    game::RunSession session;
    session.start(77u, 100u);

    TEST_ASSERT_EQUAL_UINT32(0u, session.scoreM());

    audio::MicClassification talk{};
    talk.state = audio::VoiceState::Talk;
    talk.intensity = 1.0f;
    for (int i = 0; i < 60; ++i) {
        session.tick(config::K_PHYS_DT, talk, &audio_out);
    }
    const uint32_t expected = static_cast<uint32_t>((session.player().world_x - 24.0) / 8.0);
    TEST_ASSERT_EQUAL_UINT32(expected, session.scoreM());
    TEST_ASSERT_TRUE(session.scoreM() > 0u);

    // Stand still -> score must not decrease
    const uint32_t prev_score = session.scoreM();
    for (int i = 0; i < 30; ++i) {
        session.tick(config::K_PHYS_DT, audio::MicClassification{}, &audio_out);
    }
    TEST_ASSERT_EQUAL_UINT32(prev_score, session.scoreM());
}

// AT-17: Run end saves coins on Void death AND Pause abort, updates highscore only if beaten, clamps to K_COINS_MAX
void test_AT17_run_end_persistence_and_clamping() {
    test::MockStorage storage;
    test::MockAudioOut audio_out;
    persistence::SaveData save{};
    save.coins = config::K_COINS_MAX - 1;
    save.highscore = 50;

    game::RunSession session;
    session.start(42u, save.highscore);
    audio::MicClassification talk{};
    talk.state = audio::VoiceState::Talk;
    talk.intensity = 1.0f;
    for (int i = 0; i < 120; ++i) {
        session.tick(config::K_PHYS_DT, talk, &audio_out);
    }

    // Score < 50 -> highscore must stay 50; coins clamped at K_COINS_MAX
    TEST_ASSERT_TRUE(session.commitToSave(save, storage));
    TEST_ASSERT_EQUAL_UINT32(50u, save.highscore);
    TEST_ASSERT_TRUE(save.coins <= config::K_COINS_MAX);

    // Subsequent commit is idempotent
    const uint32_t saved_coins = save.coins;
    session.commitToSave(save, storage);
    TEST_ASSERT_EQUAL_UINT32(saved_coins, save.coins);
}

// AT-18: Shop rules
void test_AT18_shop_logic() {
    test::MockStorage storage;
    persistence::SaveData save{};
    save.coins = 100;
    save.sanitize();

    TEST_ASSERT_TRUE(save.isUnlocked(persistence::ChickenId::Henni));
    TEST_ASSERT_FALSE(save.isUnlocked(persistence::ChickenId::Goldie));
    TEST_ASSERT_FALSE(save.isUnlocked(persistence::ChickenId::Ninja));

    // Cannot buy Ninja (150) with 100 coins
    TEST_ASSERT_FALSE(persistence::ShopLogic::canAfford(save, persistence::ChickenId::Ninja));
    TEST_ASSERT_FALSE(persistence::ShopLogic::buy(save, persistence::ChickenId::Ninja, storage));
    TEST_ASSERT_EQUAL_UINT32(100u, save.coins);

    // Buy Goldie (50) -> coins=50, unlocked, selected=1, saved
    TEST_ASSERT_TRUE(persistence::ShopLogic::canAfford(save, persistence::ChickenId::Goldie));
    TEST_ASSERT_TRUE(persistence::ShopLogic::buy(save, persistence::ChickenId::Goldie, storage));
    TEST_ASSERT_EQUAL_UINT32(50u, save.coins);
    TEST_ASSERT_TRUE(save.isUnlocked(persistence::ChickenId::Goldie));
    TEST_ASSERT_EQUAL_UINT8(1u, save.selected);
    TEST_ASSERT_EQUAL(1, storage.save_calls);

    // Cannot re-buy Goldie
    TEST_ASSERT_FALSE(persistence::ShopLogic::buy(save, persistence::ChickenId::Goldie, storage));
    TEST_ASSERT_EQUAL_UINT32(50u, save.coins);
}

// AT-19: Persistence validation / sanitize
void test_AT19_save_data_sanitize() {
    persistence::SaveData s{};
    s.schema_version = 99;
    s.coins = config::K_COINS_MAX + 500;
    s.highscore = config::K_COINS_MAX + 1;
    s.unlocked = 0b010; // Bit 0 missing!
    s.selected = 2;     // Ninja not unlocked!
    s.sound_on = false;

    s.sanitize();
    TEST_ASSERT_EQUAL_UINT8(config::K_SAVE_SCHEMA, s.schema_version);
    TEST_ASSERT_EQUAL_UINT32(0u, s.coins);
    TEST_ASSERT_EQUAL_UINT32(0u, s.highscore);
    TEST_ASSERT_EQUAL_UINT8(0b011u, s.unlocked); // Bit 0 forced, Bit 1 kept
    TEST_ASSERT_EQUAL_UINT8(0u, s.selected);     // Fallback to 0
    TEST_ASSERT_FALSE(s.sound_on);
}

// AT-20: InputMapper short/long press, tap distance/time, lockout 250ms / 1000ms
void test_AT20_input_mapper() {
    input::InputMapper mapper;
    hal::PlatformInput in{};

    // Short press (< 600ms) fires on release
    in.btn_left_down = true;
    auto ev = mapper.update(in, 20);
    TEST_ASSERT_FALSE(ev.left_short || ev.left_long);
    ev = mapper.update(in, 200);
    TEST_ASSERT_FALSE(ev.left_short || ev.left_long);
    in.btn_left_down = false;
    ev = mapper.update(in, 20);
    TEST_ASSERT_TRUE(ev.left_short);
    TEST_ASSERT_FALSE(ev.left_long);

    // Long press (>= 600ms) fires immediately on reaching 600ms, NO short event on release
    in.btn_right_down = true;
    ev = mapper.update(in, 20);
    TEST_ASSERT_FALSE(ev.right_long);
    ev = mapper.update(in, 600);
    TEST_ASSERT_TRUE(ev.right_long);
    in.btn_right_down = false;
    ev = mapper.update(in, 20);
    TEST_ASSERT_FALSE(ev.right_short || ev.right_long);

    // Tap within 500ms & < 10 px
    in.touch_down = true;
    in.touch_x = 50;
    in.touch_y = 60;
    mapper.update(in, 20);
    in.touch_x = 55;
    in.touch_y = 63;
    mapper.update(in, 100);
    in.touch_down = false;
    ev = mapper.update(in, 20);
    TEST_ASSERT_TRUE(ev.tap_valid);

    // Tap drag >= 10 px -> invalid
    in.touch_down = true;
    in.touch_x = 50;
    in.touch_y = 60;
    mapper.update(in, 20);
    in.touch_x = 65;
    mapper.update(in, 50);
    in.touch_down = false;
    ev = mapper.update(in, 20);
    TEST_ASSERT_FALSE(ev.tap_valid);

    // Screen lockout 1000ms + held button suppressed after transition
    in.btn_left_down = true;
    mapper.update(in, 20); // held before screen change
    mapper.onScreenChanged(1000);
    TEST_ASSERT_TRUE(mapper.isLockedOut());
    ev = mapper.update(in, 500);
    TEST_ASSERT_TRUE(mapper.isLockedOut());
    TEST_ASSERT_FALSE(ev.left_long || ev.left_short);
    ev = mapper.update(in, 550); // lockout expires now, but btn_left still held from before!
    TEST_ASSERT_FALSE(mapper.isLockedOut());
    in.btn_left_down = false;
    ev = mapper.update(in, 20); // releasing held key must NOT fire
    TEST_ASSERT_FALSE(ev.left_short);

    // Simultaneous press of both hardware buttons must suppress short & long actions until clean release
    in.btn_left_down = true;
    in.btn_right_down = true;
    ev = mapper.update(in, 20);
    ev = mapper.update(in, 650);
    TEST_ASSERT_FALSE(ev.left_long || ev.right_long || ev.left_short || ev.right_short);
    in.btn_left_down = false;
    in.btn_right_down = false;
    ev = mapper.update(in, 20);
    TEST_ASSERT_FALSE(ev.left_short || ev.right_short || ev.left_long || ev.right_long);
}

// AT-21: Complete AppController screen flow & HAL checks
void test_AT21_app_controller_screen_flow() {
    test::MockStorage storage;
    storage.has_data = true;
    storage.stored.coins = 60;
    storage.stored.highscore = 10;
    storage.stored.unlocked = 0b001;
    storage.stored.selected = 0;
    storage.stored.sound_on = true;

    test::MockAudioOut audio_out;
    test::MockHaptics haptics;
    audio::MicClassifier mic;
    app::AppController app;
    hal::Canvas155 canvas;

    app.init(storage, audio_out, haptics, mic, true, 42u);
    TEST_ASSERT_EQUAL(app::AppState::Splash, app.state());
    app.render(canvas);

    // Wait out 1500ms splash -> MainMenu
    hal::PlatformInput idle{};
    for (int i = 0; i < 80; ++i) {
        app.tick(20, idle);
    }
    TEST_ASSERT_EQUAL(app::AppState::MainMenu, app.state());
    // Wait out 250ms lockout
    for (int i = 0; i < 15; ++i) app.tick(20, idle);

    // Short right -> browse to Goldie (index 1); save.selected remains 0
    hal::PlatformInput in{};
    in.btn_right_down = true;
    app.tick(20, in);
    in.btn_right_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(persistence::ChickenId::Goldie, app.carouselChicken());
    TEST_ASSERT_EQUAL_UINT8(0u, app.saveData().selected);

    // Long right -> BuyDialog
    in.btn_right_down = true;
    app.tick(20, in);
    app.tick(620, in);
    TEST_ASSERT_EQUAL(app::AppState::BuyDialog, app.state());
    app.render(canvas);
    in.btn_right_down = false;
    app.tick(20, in);
    for (int i = 0; i < 15; ++i) app.tick(20, idle);

    // Short right -> JA (confirm purchase)
    in.btn_right_down = true;
    app.tick(20, in);
    in.btn_right_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::MainMenu, app.state());
    TEST_ASSERT_EQUAL_UINT32(10u, app.saveData().coins);
    TEST_ASSERT_TRUE(app.saveData().isUnlocked(persistence::ChickenId::Goldie));
    TEST_ASSERT_EQUAL_UINT8(1u, app.saveData().selected);
    TEST_ASSERT_FALSE(haptics.vibrations.empty());
    TEST_ASSERT_EQUAL_UINT16(config::K_HAPTIC_BUY_MS, haptics.vibrations.back());

    // Wait lockout, long right -> Start run -> Countdown -> Playing
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_right_down = true;
    app.tick(20, in);
    app.tick(620, in);
    TEST_ASSERT_EQUAL(app::AppState::Countdown, app.state());
    in.btn_right_down = false;

    // Wait pre-calib (600ms) + calib (1500ms)
    for (int i = 0; i < 120; ++i) {
        mic.processFrameDb(-60.0f, 0.02f);
        app.tick(20, idle);
    }
    TEST_ASSERT_EQUAL(app::AppState::Playing, app.state());
    app.render(canvas);

    // Pause via left short
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_left_down = true;
    app.tick(20, in);
    in.btn_left_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::Paused, app.state());
    app.render(canvas);

    // Resume via right short -> ResumeCountdown -> Playing
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_right_down = true;
    app.tick(20, in);
    in.btn_right_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::ResumeCountdown, app.state());
    for (int i = 0; i < 60; ++i) app.tick(20, idle);
    TEST_ASSERT_EQUAL(app::AppState::Playing, app.state());

    // Pause -> Menu aborts run cleanly and commits save
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_left_down = true;
    app.tick(20, in);
    in.btn_left_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::Paused, app.state());
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_left_down = true;
    app.tick(20, in);
    in.btn_left_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::MainMenu, app.state());

    // Start another run and play with sim_shout until Void death -> GameOver -> Retry
    for (int i = 0; i < 15; ++i) app.tick(20, idle);
    in.btn_right_down = true;
    app.tick(20, in);
    app.tick(620, in);
    in.btn_right_down = false;
    for (int i = 0; i < 120; ++i) {
        mic.processFrameDb(-60.0f, 0.02f);
        app.tick(20, idle);
    }
    TEST_ASSERT_EQUAL(app::AppState::Playing, app.state());

    // Run until Void death: walk with sim_talk, do a 200ms shout if blocked by an obstacle
    bool reached_gameover = false;
    int shout_burst = 0;
    for (int i = 0; i < 2000; ++i) {
        hal::PlatformInput sim_run{};
        if (shout_burst > 0) {
            sim_run.sim_shout = true;
            --shout_burst;
        } else if (i > 10 && app.runSession().player().vx < 1.0f && app.runSession().player().on_ground) {
            sim_run.sim_shout = true;
            shout_burst = 10;
        } else {
            sim_run.sim_talk = true;
        }
        app.tick(20, sim_run);
        if (app.state() == app::AppState::GameOver) {
            reached_gameover = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(reached_gameover);
    app.render(canvas);

    // Wait 1000ms GameOver lockout, then NOCHMAL (right short) -> Countdown
    for (int i = 0; i < 55; ++i) app.tick(20, idle);
    in.btn_right_down = true;
    app.tick(20, in);
    in.btn_right_down = false;
    app.tick(20, in);
    TEST_ASSERT_EQUAL(app::AppState::Countdown, app.state());
}

void test_UI_pause_gameover_hitboxes_and_safe_circle() {
    auto cornerInsideSafe = [](int x, int y) {
        const int dx = x - config::K_SAFE_CENTER_X;
        const int dy = y - config::K_SAFE_CENTER_Y;
        return (dx * dx + dy * dy) <= (config::K_SAFE_RADIUS * config::K_SAFE_RADIUS);
    };
    auto rectInsideSafe = [&](const core::RectI& r) {
        return cornerInsideSafe(r.x, r.y) &&
               cornerInsideSafe(r.x + r.w - 1, r.y) &&
               cornerInsideSafe(r.x, r.y + r.h - 1) &&
               cornerInsideSafe(r.x + r.w - 1, r.y + r.h - 1);
    };
    auto rectsOverlap = [](const core::RectI& a, const core::RectI& b) {
        return a.x < (b.x + b.w) && (a.x + a.w) > b.x &&
               a.y < (b.y + b.h) && (a.y + a.h) > b.y;
    };

    // Visual button rects fit inside Safe-Circle (R=72 around 77,77)
    TEST_ASSERT_TRUE(rectInsideSafe(render::UILayout::kPauseBtnMenuVis));
    TEST_ASSERT_TRUE(rectInsideSafe(render::UILayout::kPauseBtnResumeVis));
    TEST_ASSERT_TRUE(rectInsideSafe(render::UILayout::kGameOverBtnMenuVis));
    TEST_ASSERT_TRUE(rectInsideSafe(render::UILayout::kGameOverBtnRetryVis));

    // Expanded hitboxes are larger than visual rects and do not overlap each other
    TEST_ASSERT_TRUE(render::UILayout::kPauseBtnMenu.w >= render::UILayout::kPauseBtnMenuVis.w);
    TEST_ASSERT_TRUE(render::UILayout::kPauseBtnMenu.h >= render::UILayout::kPauseBtnMenuVis.h);
    TEST_ASSERT_TRUE(render::UILayout::kGameOverBtnMenu.w >= render::UILayout::kGameOverBtnMenuVis.w);
    TEST_ASSERT_TRUE(render::UILayout::kGameOverBtnRetry.h >= render::UILayout::kGameOverBtnRetryVis.h);
    TEST_ASSERT_FALSE(rectsOverlap(render::UILayout::kPauseBtnMenu, render::UILayout::kPauseBtnResume));
    TEST_ASSERT_FALSE(rectsOverlap(render::UILayout::kGameOverBtnMenu, render::UILayout::kGameOverBtnRetry));
}

void test_FR_BM_renderer_transitions_and_ambient() {
    hal::Canvas155 canvas_green;
    hal::Canvas155 canvas_lava;
    hal::Canvas155 canvas_blend;

    render::WorldRenderer::renderAtmosphere(
        canvas_green, 0.0, world::BiomeId::Greenland, world::BiomeId::Greenland, 1.0f, 0.5f);
    render::WorldRenderer::renderAtmosphere(
        canvas_lava, 0.0, world::BiomeId::Lava, world::BiomeId::Lava, 1.0f, 0.5f);
    render::WorldRenderer::renderAtmosphere(
        canvas_blend, 0.0, world::BiomeId::Greenland, world::BiomeId::Lava, 0.5f, 0.5f);

    // Mid-transition contains pixels from both Greenland and Lava atmospheres (Bayer dither)
    bool has_green_sky = false;
    bool has_lava_sky = false;
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < hal::Canvas155::kSize; ++x) {
            const uint8_t px = canvas_blend.getPixel(x, y);
            if (px == render::C_LIGHT_BLUE) has_green_sky = true;
            if (px == render::C_BLACK) has_lava_sky = true;
        }
    }
    TEST_ASSERT_TRUE(has_green_sky);
    TEST_ASSERT_TRUE(has_lava_sky);

    // Lava bottom void renders animated lava river colors (Red/Orange/Yellow)
    bool has_lava_river = false;
    for (int x = 0; x < hal::Canvas155::kSize; ++x) {
        const uint8_t px = canvas_lava.getPixel(x, 145);
        if (px == render::C_RED || px == render::C_ORANGE || px == render::C_YELLOW) {
            has_lava_river = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(has_lava_river);
}

void test_FR_PK_candy_mode_and_biome_renderers() {
    // 1. Verify Snow iceberg silhouette, Jungle, Sand, and Candy atmospheres render cleanly
    hal::Canvas155 c_snow, c_jungle, c_sand, c_candy0, c_candy1;
    render::WorldRenderer::renderAtmosphere(
        c_snow, 0.0, world::BiomeId::Snow, world::BiomeId::Snow, 1.0f, 0.5f);
    render::WorldRenderer::renderAtmosphere(
        c_jungle, 0.0, world::BiomeId::Jungle, world::BiomeId::Jungle, 1.0f, 0.5f);
    render::WorldRenderer::renderAtmosphere(
        c_sand, 0.0, world::BiomeId::Sand, world::BiomeId::Sand, 1.0f, 0.5f);
    render::WorldRenderer::renderAtmosphere(
        c_candy0, 0.0, world::BiomeId::Candy, world::BiomeId::Candy, 1.0f, 0.05f);
    render::WorldRenderer::renderAtmosphere(
        c_candy1, 0.0, world::BiomeId::Candy, world::BiomeId::Candy, 1.0f, 0.55f);

    bool snow_ice = false;
    bool sand_sun = false;
    bool jungle_canopy = false;
    bool candy_colors = false;
    bool candy_flashes = false;
    for (int y = 5; y < 120; ++y) {
        for (int x = 0; x < hal::Canvas155::kSize; ++x) {
            if (c_snow.getPixel(x, y) == render::C_LIGHT_BLUE || c_snow.getPixel(x, y) == render::C_WHITE) {
                snow_ice = true;
            }
            if (y < 40 && (c_sand.getPixel(x, y) == render::C_YELLOW || c_sand.getPixel(x, y) == render::C_WHITE)) {
                sand_sun = true;
            }
            if (c_jungle.getPixel(x, y) == render::C_DARK_GREEN || c_jungle.getPixel(x, y) == render::C_GREEN) {
                jungle_canopy = true;
            }
            const uint8_t p0 = c_candy0.getPixel(x, y);
            const uint8_t p1 = c_candy1.getPixel(x, y);
            if (p0 == render::C_DARK_PURPLE || p0 == render::C_PINK || p0 == render::C_LAVENDER) {
                candy_colors = true;
            }
            if (p0 != p1) {
                candy_flashes = true;
            }
        }
    }
    TEST_ASSERT_TRUE(snow_ice);
    TEST_ASSERT_TRUE(sand_sun);
    TEST_ASSERT_TRUE(jungle_canopy);
    TEST_ASSERT_TRUE(candy_colors);
    TEST_ASSERT_TRUE(candy_flashes);

    // 2. Drive a RunSession until picking up a white-packet Perk, verify Candy + expiry restore
    game::RunSession session;
    session.start(42u, 0u);
    test::MockAudioOut audio;

    bool triggered_candy = false;
    bool restored_biome  = false;
    bool heard_perk_sfx  = false;
    for (int frame = 0; frame < 4500; ++frame) {
        audio::MicClassification voice{};
        voice.state = audio::VoiceState::Talk;
        voice.intensity = 0.85f;

        // Simple lookahead bot to navigate across islands/obstacles toward a PerkItem
        const double px = session.player().world_x;
        bool need_jump = false;
        if (!session.world().perks.empty()) {
            const auto& pk = session.world().perks[0];
            if (!pk.collected && pk.x > px - 6.0 && pk.x < px + 36.0 &&
                session.player().y > pk.y + 4.0f) {
                need_jump = true;
            }
        }
        for (std::size_t i = 0; i < session.world().obstacles.size(); ++i) {
            const auto& o = session.world().obstacles[i];
            if (o.type != world::ObstacleType::Overhang &&
                o.x > px + 2.0 && o.x < px + 22.0) {
                need_jump = true;
                break;
            }
        }
        bool on_safe_span = false;
        for (std::size_t i = 0; i < session.world().islands.size(); ++i) {
            const auto& isl = session.world().islands[i];
            if (px + 18.0 >= isl.x && px + 18.0 <= isl.x + isl.w) {
                on_safe_span = true;
                break;
            }
        }
        if (!on_safe_span) {
            need_jump = true;
        }
        if (need_jump && session.player().perk_timer <= 0.0f) {
            voice.state = audio::VoiceState::Shout;
        }

        const bool ended = session.tick(config::K_PHYS_DT, voice, &audio);
        if (ended) {
            break;
        }
        if (session.player().perk_timer > 0.0f) {
            triggered_candy = true;
            TEST_ASSERT_EQUAL(static_cast<uint8_t>(world::BiomeId::Candy),
                              static_cast<uint8_t>(session.currentBiome()));
            for (const auto s : audio.played_sounds) {
                if (s == audio::SoundId::Perk) {
                    heard_perk_sfx = true;
                    break;
                }
            }
            TEST_ASSERT_TRUE(session.player().y >= config::K_PERK_CEILING_Y - 0.01f);
        } else if (triggered_candy) {
            // Once perk_timer reaches 0, currentBiome must leave Candy
            TEST_ASSERT_TRUE(session.currentBiome() != world::BiomeId::Candy);
            restored_biome = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(triggered_candy);
    TEST_ASSERT_TRUE(heard_perk_sfx);
    TEST_ASSERT_TRUE(restored_biome);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_AT16_score_calculation_monotonic);
    RUN_TEST(test_AT17_run_end_persistence_and_clamping);
    RUN_TEST(test_AT18_shop_logic);
    RUN_TEST(test_AT19_save_data_sanitize);
    RUN_TEST(test_AT20_input_mapper);
    RUN_TEST(test_AT21_app_controller_screen_flow);
    RUN_TEST(test_UI_pause_gameover_hitboxes_and_safe_circle);
    RUN_TEST(test_FR_BM_renderer_transitions_and_ambient);
    RUN_TEST(test_FR_PK_candy_mode_and_biome_renderers);
    return UNITY_END();
}
