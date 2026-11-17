#include <unity.h>
#include <cmath>

#include "mock_hal.h"
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/game/physics.h"
#include "schreihuhn/game/run_session.h"
#include "schreihuhn/world/world_types.h"

using namespace schreihuhn;

void setUp() {}
void tearDown() {}

// AT-06: Physics reference values: height 33+-2, unflapped dist 47+-3, flapped dist 100+-8
void test_AT06_physics_reference_jump_and_flap() {
    auto measureJump = [](bool with_flap, float& out_height, float& out_dist) {
        world::WorldState world;
        world.islands.pushBack(world::Island{0.0, 300.0f, config::K_LEVEL_0});
        game::PlayerState player;
        player.reset();

        float min_y = player.y;
        audio::MicClassification shout{};
        shout.state = audio::VoiceState::Shout;
        audio::MicClassification still{};
        still.state = audio::VoiceState::Still;

        // Tick 0: jump
        game::Physics::step(player, world, shout, config::K_PHYS_DT);
        if (player.y < min_y) min_y = player.y;

        for (int t = 1; t < 300; ++t) {
            game::Physics::step(player, world, with_flap ? shout : still, config::K_PHYS_DT);
            if (player.y < min_y) min_y = player.y;
            if (player.on_ground) {
                break;
            }
        }
        out_height = (config::K_LEVEL_0 - config::K_HITBOX_H) - min_y;
        out_dist = static_cast<float>(player.world_x - config::K_START_X);
    };

    float h_no_flap = 0.0f, d_no_flap = 0.0f;
    measureJump(false, h_no_flap, d_no_flap);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 33.0f, h_no_flap);
    TEST_ASSERT_FLOAT_WITHIN(3.0f, 47.0f, d_no_flap);

    float h_flap = 0.0f, d_flap = 0.0f;
    measureJump(true, h_flap, d_flap);
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 33.0f, h_flap);
    TEST_ASSERT_FLOAT_WITHIN(8.0f, 100.0f, d_flap);
}

// AT-07: Framerate independence (30 FPS vs 60 FPS -> |x_30 - x_60| <= 1 px)
void test_AT07_framerate_independence() {
    auto runAtFps = [](int fps) {
        test::MockAudioOut audio_out;
        game::RunSession session;
        session.start(42u, 0u);
        const float dt = 1.0f / static_cast<float>(fps);
        const int total_frames = fps * 5; // 5 seconds

        for (int f = 0; f < total_frames; ++f) {
            const float t = static_cast<float>(f) * dt;
            audio::MicClassification voice{};
            if (t < 1.5f) {
                voice.state = audio::VoiceState::Talk;
                voice.intensity = 0.6f;
            } else if (t < 2.5f) {
                voice.state = audio::VoiceState::Shout;
            } else if (t < 3.5f) {
                voice.state = audio::VoiceState::Still;
            } else {
                voice.state = audio::VoiceState::Talk;
                voice.intensity = 1.0f;
            }
            session.tick(dt, voice, &audio_out);
        }
        return static_cast<float>(session.player().world_x);
    };

    const float x30 = runAtFps(30);
    const float x60 = runAtFps(60);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, x60, x30);
}

// AT-08: Stamina consumed only when flapping & descending, reset on landing
void test_AT08_flap_stamina() {
    world::WorldState world;
    world.islands.pushBack(world::Island{0.0, 300.0f, config::K_LEVEL_0});
    game::PlayerState player;
    player.reset();

    audio::MicClassification shout{};
    shout.state = audio::VoiceState::Shout;

    // Jump
    game::Physics::step(player, world, shout, config::K_PHYS_DT);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, config::K_FLAP_STAMINA, player.stamina);

    // While rising (vy < 0), stamina must NOT decrease
    bool saw_descent = false;
    for (int t = 0; t < 120; ++t) {
        game::Physics::step(player, world, shout, config::K_PHYS_DT);
        if (player.vy < 0.0f) {
            TEST_ASSERT_FLOAT_WITHIN(0.001f, config::K_FLAP_STAMINA, player.stamina);
        } else if (!player.on_ground && !saw_descent) {
            saw_descent = true;
            TEST_ASSERT_TRUE(player.stamina < config::K_FLAP_STAMINA);
        }
        if (player.on_ground) {
            // Reset upon landing
            TEST_ASSERT_FLOAT_WITHIN(0.001f, config::K_FLAP_STAMINA, player.stamina);
            break;
        }
    }
    TEST_ASSERT_TRUE(saw_descent);
    TEST_ASSERT_TRUE(player.on_ground);
}

// AT-09: Collision (lateral stop without death, stand on obstacle, head vy=0, no tunneling)
void test_AT09_collision_and_anti_tunneling() {
    world::WorldState world;
    world.islands.pushBack(world::Island{0.0, 120.0f, config::K_LEVEL_0});
    world.obstacles.pushBack(world::Obstacle{40.0, config::K_LEVEL_0 - 16.0f, 16.0f, 16.0f,
                                             world::ObstacleType::Stone, -1, false, 0.0f});

    game::PlayerState player;
    player.reset();

    audio::MicClassification talk{};
    talk.state = audio::VoiceState::Talk;
    talk.intensity = 1.0f;

    // Walk into stone block at x=40 -> stops at x=30 (40 - K_HITBOX_W) with vx=0
    for (int i = 0; i < 60; ++i) {
        game::Physics::step(player, world, talk, config::K_PHYS_DT);
    }
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, static_cast<float>(player.world_x));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, player.vx);

    // Stand on top of obstacle
    player.world_x = 42.0;
    player.y = config::K_LEVEL_0 - 16.0f - config::K_HITBOX_H - 2.0f;
    player.vy = 20.0f;
    player.on_ground = false;
    for (int i = 0; i < 10; ++i) {
        game::Physics::step(player, world, audio::MicClassification{}, config::K_PHYS_DT);
    }
    TEST_ASSERT_TRUE(player.on_ground);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, config::K_LEVEL_0 - 16.0f - config::K_HITBOX_H, player.y);

    // Head collision sets vy = 0
    world.clear();
    world.islands.pushBack(world::Island{0.0, 120.0f, config::K_LEVEL_0});
    world.obstacles.pushBack(world::Obstacle{20.0, config::K_LEVEL_0 - 30.0f, 20.0f, 10.0f,
                                             world::ObstacleType::Stone, -1, false, 0.0f});
    player.reset();
    player.y = config::K_LEVEL_0 - 19.0f;
    player.vy = -120.0f;
    player.on_ground = false;
    game::Physics::step(player, world, audio::MicClassification{}, config::K_PHYS_DT);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, player.vy);

    // Anti-tunneling at K_MAX_FALL (260 px/s) onto 4 px thin bridge segment
    world.clear();
    world.bridges.pushBack(world::BridgeSegment{20.0, 104.0f, config::K_BRIDGE_STABILITY, true, 0.0f});
    player.reset();
    player.world_x = 22.0;
    player.y = 104.0f - config::K_HITBOX_H - 1.0f;
    player.vy = config::K_MAX_FALL;
    player.on_ground = false;
    game::Physics::step(player, world, audio::MicClassification{}, config::K_PHYS_DT);
    TEST_ASSERT_TRUE(player.on_ground);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 104.0f - config::K_HITBOX_H, player.y);
}

// AT-10: Coyote time: jump 70 ms after edge works, 90 ms fails
void test_AT10_coyote_time() {
    auto setupAfterEdge = [](float fall_sec) {
        world::WorldState world;
        world.islands.pushBack(world::Island{0.0, 30.0f, config::K_LEVEL_0});
        game::PlayerState player;
        player.reset();
        player.world_x = 35.0; // past island right edge
        player.on_ground = true;
        player.coyote_timer = config::K_COYOTE;

        audio::MicClassification still{};
        const int steps = static_cast<int>(std::round(fall_sec / config::K_PHYS_DT));
        for (int i = 0; i < steps; ++i) {
            game::Physics::step(player, world, still, config::K_PHYS_DT);
        }
        audio::MicClassification shout{};
        shout.state = audio::VoiceState::Shout;
        return game::Physics::step(player, world, shout, config::K_PHYS_DT);
    };

    // ~67 ms (4 ticks) < 80 ms -> jump succeeds
    const auto r70 = setupAfterEdge(0.067f);
    TEST_ASSERT_TRUE(r70.jumped_this_tick);

    // ~100 ms (6 ticks) > 80 ms -> jump fails
    const auto r90 = setupAfterEdge(0.100f);
    TEST_ASSERT_FALSE(r90.jumped_this_tick);
}

// AT-11: Void death when hitbox top > 155
void test_AT11_void_death() {
    test::MockAudioOut audio_out;
    game::RunSession session;
    session.start(123u, 0u);

    // Walk off the 96px start island onto a guaranteed gap? Or use a direct Physics test + RunSession
    // Test Physics died_void directly first:
    world::WorldState world;
    game::PlayerState player;
    player.reset();
    player.y = 154.0f;
    player.vy = 200.0f;
    player.on_ground = false;
    const auto res = game::Physics::step(player, world, audio::MicClassification{}, config::K_PHYS_DT);
    TEST_ASSERT_TRUE(res.died_void);
    TEST_ASSERT_TRUE(player.y > config::K_VOID_Y);
}

// AT-12: Bridge stability, shake threshold, collapse, no regeneration, walkable at K_WALK_MIN
void test_AT12_bridge_mechanics() {
    // Spec invariant FR-MP-16
    const float cross_time = (config::K_BRIDGE_SEG_W + config::K_HITBOX_W) / config::K_WALK_MIN;
    TEST_ASSERT_TRUE(cross_time < config::K_BRIDGE_STABILITY);

    // Crossing at K_WALK_MIN across 6 segments -> none collapse under chicken
    {
        world::WorldState world;
        world.islands.pushBack(world::Island{0.0, 40.0f, config::K_LEVEL_0});
        for (int i = 0; i < 6; ++i) {
            world.bridges.pushBack(world::BridgeSegment{
                40.0 + i * 8.0, config::K_LEVEL_0, config::K_BRIDGE_STABILITY, true, 0.0f});
        }
        world.islands.pushBack(world::Island{88.0, 60.0f, config::K_LEVEL_0});

        game::PlayerState player;
        player.reset();
        player.world_x = 24.0;
        player.vx = config::K_WALK_MIN;

        audio::MicClassification walk_min{};
        walk_min.state = audio::VoiceState::Talk;
        walk_min.intensity = 0.0f;

        for (int t = 0; t < 200; ++t) {
            game::Physics::step(player, world, walk_min, config::K_PHYS_DT);
            TEST_ASSERT_TRUE(player.on_ground);
            if (player.world_x > 95.0) break;
        }
        TEST_ASSERT_TRUE(player.world_x > 95.0);
        // Check all segments stayed intact while chicken was on them
        for (std::size_t i = 0; i < world.bridges.size(); ++i) {
            TEST_ASSERT_TRUE(world.bridges[i].intact);
        }
    }

    // Standing still: shakes at < 0.35s, collapses at 0.7s, no regeneration
    {
        world::WorldState world;
        world.bridges.pushBack(world::BridgeSegment{
            20.0, config::K_LEVEL_0, config::K_BRIDGE_STABILITY, true, 0.0f});
        game::PlayerState player;
        player.reset();
        player.world_x = 20.0;

        audio::MicClassification still{};
        // Step 0.40s -> remaining stability ~0.30s < K_BRIDGE_SHAKE (0.35s), still intact
        for (int t = 0; t < 24; ++t) {
            game::Physics::step(player, world, still, config::K_PHYS_DT);
        }
        TEST_ASSERT_TRUE(world.bridges[0].intact);
        TEST_ASSERT_TRUE(world.bridges[0].stability < config::K_BRIDGE_SHAKE);

        // Step off bridge for 1.0s -> stability must NOT regenerate
        const float saved_stab = world.bridges[0].stability;
        player.world_x = 100.0; // off bridge
        for (int t = 0; t < 60; ++t) {
            game::Physics::step(player, world, still, config::K_PHYS_DT);
        }
        TEST_ASSERT_FLOAT_WITHIN(0.001f, saved_stab, world.bridges[0].stability);

        // Step back on and finish remaining ~0.30s -> collapses
        player.world_x = 20.0;
        player.y = config::K_LEVEL_0 - config::K_HITBOX_H;
        player.vy = 0.0f;
        player.on_ground = true;
        for (int t = 0; t < 25; ++t) {
            game::Physics::step(player, world, still, config::K_PHYS_DT);
        }
        TEST_ASSERT_FALSE(world.bridges[0].intact);
    }
}

void test_FR_OB_overhang_walk_under_vs_jump_headhit() {
    world::WorldState world;
    world.islands.pushBack(world::Island{0.0, 140.0f, config::K_LEVEL_0});
    // Overhang 24x12 floating K_OVERHANG_CLEAR_PX (20 px) above K_LEVEL_0
    world.obstacles.pushBack(world::Obstacle{
        50.0, config::K_LEVEL_0 - config::K_OVERHANG_CLEAR_PX - 12.0f,
        24.0f, 12.0f, world::ObstacleType::Overhang});

    game::PlayerState player;
    player.reset();
    player.world_x = 30.0;

    audio::MicClassification talk{};
    talk.state = audio::VoiceState::Talk;
    talk.intensity = 1.0f;

    // 1. Calm TALK walks cleanly underneath without hitting any wall
    for (int t = 0; t < 90; ++t) {
        const auto res = game::Physics::step(player, world, talk, config::K_PHYS_DT);
        TEST_ASSERT_FALSE(res.hit_wall);
    }
    TEST_ASSERT_TRUE(player.world_x > 80.0);
    TEST_ASSERT_TRUE(player.on_ground);

    // 2. Jumping directly underneath bumps head on overhang bottom (y=K_LEVEL_0-20)
    player.reset();
    player.world_x = 55.0;
    audio::MicClassification shout{};
    shout.state = audio::VoiceState::Shout;
    bool hit_head_bounce = false;
    for (int t = 0; t < 20; ++t) {
        game::Physics::step(player, world, (t == 0) ? shout : talk, config::K_PHYS_DT);
        if (t > 0 && player.y >= config::K_LEVEL_0 - config::K_OVERHANG_CLEAR_PX - 0.1f && player.vy == 0.0f) {
            hit_head_bounce = true;
            break;
        }
    }
    TEST_ASSERT_TRUE(hit_head_bounce);
    TEST_ASSERT_TRUE(player.y >= config::K_LEVEL_0 - config::K_OVERHANG_CLEAR_PX - 0.1f);
}

void test_FR_PK_flight_physics_and_ceiling_barrier() {
    world::WorldState world;
    world.islands.pushBack(world::Island{0.0, 2000.0f, config::K_LEVEL_0, world::BiomeId::Candy});

    game::PlayerState player;
    player.reset();
    player.world_x = 40.0;
    player.perk_timer = config::K_PERK_DURATION_S;

    audio::MicClassification talk{};
    talk.state = audio::VoiceState::Talk;
    talk.intensity = 0.6f;

    // 1. Calm TALK triggers effortless jump + upward flight without needing SHOUT
    const auto jump_res = game::Physics::step(player, world, talk, config::K_PHYS_DT);
    TEST_ASSERT_TRUE(jump_res.jumped_this_tick);
    TEST_ASSERT_FALSE(player.on_ground);
    TEST_ASSERT_TRUE(player.vy < 0.0f);

    // 2. Sustained TALK climbs steadily toward ceiling and stops safely at K_PERK_CEILING_Y
    for (int i = 0; i < 120; ++i) {
        game::Physics::step(player, world, talk, config::K_PHYS_DT);
        TEST_ASSERT_TRUE(player.y >= config::K_PERK_CEILING_Y - 0.001f);
        TEST_ASSERT_FLOAT_WITHIN(0.01f, config::K_FLAP_STAMINA, player.stamina);
    }
    TEST_ASSERT_TRUE(player.y <= config::K_PERK_CEILING_Y + 0.5f);

    // 3. Releasing voice (STILL) glides softly down at <= K_PERK_MAX_FALL
    audio::MicClassification still{};
    still.state = audio::VoiceState::Still;
    for (int i = 0; i < 40; ++i) {
        game::Physics::step(player, world, still, config::K_PHYS_DT);
        TEST_ASSERT_TRUE(player.vy <= config::K_PERK_MAX_FALL + 0.01f);
    }
    TEST_ASSERT_TRUE(player.y > config::K_PERK_CEILING_Y + 2.0f);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_AT06_physics_reference_jump_and_flap);
    RUN_TEST(test_AT07_framerate_independence);
    RUN_TEST(test_AT08_flap_stamina);
    RUN_TEST(test_AT09_collision_and_anti_tunneling);
    RUN_TEST(test_AT10_coyote_time);
    RUN_TEST(test_AT11_void_death);
    RUN_TEST(test_AT12_bridge_mechanics);
    RUN_TEST(test_FR_OB_overhang_walk_under_vs_jump_headhit);
    RUN_TEST(test_FR_PK_flight_physics_and_ceiling_barrier);
    return UNITY_END();
}
