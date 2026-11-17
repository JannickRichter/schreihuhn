#include <unity.h>
#include <cmath>
#include <vector>

#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/game/physics.h"
#include "schreihuhn/world/world_types.h"

using namespace schreihuhn;

void setUp() {}
void tearDown() {}

// AT-02: Mic classification, hysteresis, K_SHOUT_MIN_FRAMES, intensity, DC filter
void test_AT02_mic_classification_and_hysteresis() {
    audio::MicClassifier mic;
    mic.beginCalibration();
    for (int i = 0; i < 75; ++i) {
        mic.processFrameDb(-60.0f, 0.02f);
    }
    mic.finishCalibration();
    // floor=-60, talk=-48, shout=-30
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -60.0f, mic.current().floor_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -48.0f, mic.current().talk_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -30.0f, mic.current().shout_db);
    TEST_ASSERT_EQUAL(audio::VoiceState::Still, mic.current().state);

    // Stay STILL below talk_db
    for (int i = 0; i < 10; ++i) {
        mic.processFrameDb(-52.0f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Still, mic.current().state);

    // Ramp into TALK
    for (int i = 0; i < 10; ++i) {
        mic.processFrameDb(-39.0f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Talk, mic.current().state);
    TEST_ASSERT_TRUE(mic.current().intensity > 0.3f && mic.current().intensity < 0.8f);

    // Hysteresis: drop slightly below talk_db (-48) to -49.5 (> -51) -> remains TALK
    for (int i = 0; i < 15; ++i) {
        mic.processFrameDb(-49.5f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Talk, mic.current().state);

    // Drop below talk_db - K_HYSTERESIS_DB (-51) -> STILL
    for (int i = 0; i < 15; ++i) {
        mic.processFrameDb(-55.0f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Still, mic.current().state);

    // K_SHOUT_MIN_FRAMES = 2: 1 frame >= shout_db is NOT enough
    mic.processFrameDb(-10.0f, 0.02f);
    TEST_ASSERT_TRUE(mic.current().smoothed_db >= mic.current().shout_db);
    TEST_ASSERT_TRUE(mic.current().state != audio::VoiceState::Shout);

    // 2nd consecutive frame >= shout_db -> SHOUT
    mic.processFrameDb(-10.0f, 0.02f);
    TEST_ASSERT_EQUAL(audio::VoiceState::Shout, mic.current().state);

    // Hysteresis on SHOUT: drop to -32 (> shout_db - 3 = -33) -> stays SHOUT
    for (int i = 0; i < 20; ++i) {
        mic.processFrameDb(-32.0f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Shout, mic.current().state);

    // Drop below -33 -> falls back to TALK
    for (int i = 0; i < 10; ++i) {
        mic.processFrameDb(-36.0f, 0.02f);
    }
    TEST_ASSERT_EQUAL(audio::VoiceState::Talk, mic.current().state);

    // DC filter test: pure DC offset (no AC signal) must yield -90 dBFS
    std::vector<int16_t> dc_samples(320, 4000);
    mic.processSamples(dc_samples.data(), dc_samples.size(), 0.02f);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -90.0f, mic.current().raw_db);
}

// AT-03: Calibration median, clamping [-70, -40], shout >= talk + 10 dB
void test_AT03_calibration_median_and_clamping() {
    auto runCalib = [](const std::vector<float>& vals) {
        audio::MicClassifier mic;
        mic.beginCalibration();
        for (float v : vals) {
            mic.processFrameDb(v, 0.02f);
        }
        mic.finishCalibration();
        return mic.current();
    };

    // Floor -80 -> clamped to -70; shout_db >= K_SHOUT_ABS_MIN_DB (-30)
    auto c80 = runCalib({-80.0f, -80.0f, -80.0f, -20.0f, -80.0f});
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -70.0f, c80.floor_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -58.0f, c80.talk_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -30.0f, c80.shout_db);
    TEST_ASSERT_TRUE(c80.shout_db >= c80.talk_db + 10.0f);

    // Floor -60 (with outliers) -> median -60
    auto c60 = runCalib({-60.0f, -20.0f, -61.0f, -59.0f, -60.0f});
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -60.0f, c60.floor_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -48.0f, c60.talk_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -30.0f, c60.shout_db);

    // Floor -45 -> talk=-33, shout=-17
    auto c45 = runCalib({-45.0f, -45.0f, -45.0f});
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -45.0f, c45.floor_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -33.0f, c45.talk_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -17.0f, c45.shout_db);

    // Floor -20 -> clamped to -40; talk=-28, shout=-12 (>= talk+10)
    auto c20 = runCalib({-20.0f, -20.0f, -20.0f});
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -40.0f, c20.floor_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -28.0f, c20.talk_db);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -12.0f, c20.shout_db);
    TEST_ASSERT_TRUE(c20.shout_db >= c20.talk_db + 10.0f);
}

// AT-04: Latency logic: Shout -> Jump <= 100 ms; Silence -> vx == 0 <= 150 ms
void test_AT04_latency_jump_and_stop() {
    audio::MicClassifier mic;
    mic.beginCalibration();
    for (int i = 0; i < 75; ++i) mic.processFrameDb(-60.0f, 0.02f);
    mic.finishCalibration();

    world::WorldState world;
    world.islands.pushBack(world::Island{0.0, 200.0f, config::K_LEVEL_0});
    game::PlayerState player;
    player.reset();

    // Test 1: Shout onset -> Jump <= 100 ms
    float elapsed_ms = 0.0f;
    bool jumped = false;
    for (int frame = 0; frame < 6; ++frame) { // 6 * 20ms = 120ms max
        mic.processFrameDb(-15.0f, 0.02f);
        elapsed_ms += 20.0f;
        const auto res = game::Physics::step(player, world, mic.current(), config::K_PHYS_DT);
        if (res.jumped_this_tick) {
            jumped = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(jumped);
    TEST_ASSERT_TRUE(elapsed_ms <= 100.0f);

    // Test 2: Talking at typical speech level -> Silence -> vx == 0 <= 150 ms
    player.reset();
    mic.finishCalibration();
    for (int i = 0; i < 15; ++i) {
        mic.processFrameDb(-44.0f, 0.02f);
        game::Physics::step(player, world, mic.current(), config::K_PHYS_DT);
    }
    TEST_ASSERT_TRUE(player.vx >= config::K_WALK_MIN * 0.9f);

    float stop_ms = 0.0f;
    bool stopped = false;
    for (int tick = 0; tick < 12; ++tick) {
        // Feed 20ms mic frames synchronously on ~20ms cadence
        if ((tick % 1) == 0) {
            mic.processFrameDb(-60.0f, config::K_PHYS_DT);
        }
        game::Physics::step(player, world, mic.current(), config::K_PHYS_DT);
        stop_ms += config::K_PHYS_DT * 1000.0f;
        if (player.vx == 0.0f) {
            stopped = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(stopped);
    TEST_ASSERT_TRUE(stop_ms <= 150.0f);
}

// AT-05: Gating freezes level/state and blocks new SHOUT transitions
void test_AT05_gating_freezes_state_and_blocks_shout() {
    audio::MicClassifier mic;
    mic.beginCalibration();
    for (int i = 0; i < 75; ++i) mic.processFrameDb(-60.0f, 0.02f);
    mic.finishCalibration();

    // Bring to TALK
    for (int i = 0; i < 10; ++i) mic.processFrameDb(-40.0f, 0.02f);
    TEST_ASSERT_EQUAL(audio::VoiceState::Talk, mic.current().state);
    const float saved_db = mic.current().smoothed_db;

    // Trigger 110ms Coin sound -> gate = 110 + 150 = 260ms
    mic.triggerSoundGate(110);
    for (int i = 0; i < 12; ++i) { // 12 * 20ms = 240ms < 260ms
        mic.processFrameDb(-5.0f, 0.02f); // loud speaker feedback!
        TEST_ASSERT_TRUE(mic.current().gated);
        TEST_ASSERT_EQUAL(audio::VoiceState::Talk, mic.current().state);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, saved_db, mic.current().smoothed_db);
    }

    // Also verify STILL stays STILL under gating
    mic.clearGate();
    for (int i = 0; i < 20; ++i) mic.processFrameDb(-65.0f, 0.02f);
    TEST_ASSERT_EQUAL(audio::VoiceState::Still, mic.current().state);
    mic.triggerSoundGate(100);
    for (int i = 0; i < 10; ++i) {
        mic.processFrameDb(-5.0f, 0.02f);
        TEST_ASSERT_EQUAL(audio::VoiceState::Still, mic.current().state);
    }

    // After gate expires (250ms total), loud shout works again
    for (int i = 0; i < 10; ++i) {
        mic.processFrameDb(-5.0f, 0.02f);
    }
    TEST_ASSERT_FALSE(mic.current().gated);
    TEST_ASSERT_EQUAL(audio::VoiceState::Shout, mic.current().state);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_AT02_mic_classification_and_hysteresis);
    RUN_TEST(test_AT03_calibration_median_and_clamping);
    RUN_TEST(test_AT04_latency_jump_and_stop);
    RUN_TEST(test_AT05_gating_freezes_state_and_blocks_shout);
    return UNITY_END();
}
