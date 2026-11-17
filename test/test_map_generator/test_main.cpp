#include <unity.h>
#include <cmath>
#include <vector>

#include "schreihuhn/config/tuning.h"
#include "schreihuhn/world/map_generator.h"
#include "schreihuhn/world/solvability.h"
#include "schreihuhn/world/world_types.h"

using namespace schreihuhn;

void setUp() {}
void tearDown() {}

// AT-13: Same seed produces bit-identical world sequences over 3000 m
void test_AT13_generator_determinism() {
    struct Snapshot {
        std::vector<double> chunk_ends;
        std::vector<double> island_x;
        std::vector<float> island_y;
        std::vector<double> coin_x;
    };
    auto runGen = [](uint32_t seed) {
        world::WorldState world;
        world::MapGenerator gen;
        gen.reset(seed, world);
        Snapshot s;
        while (gen.cursorX() < 24024.0) {
            const auto ch = gen.generateNextChunk(world);
            s.chunk_ends.push_back(ch.end_x);
            for (std::size_t i = 0; i < world.islands.size(); ++i) {
                s.island_x.push_back(world.islands[i].x);
                s.island_y.push_back(world.islands[i].y);
            }
            for (std::size_t i = 0; i < world.coins.size(); ++i) {
                s.coin_x.push_back(world.coins[i].x);
            }
            gen.pruneBefore(world, gen.cursorX() - 400.0);
        }
        return s;
    };

    const auto a = runGen(42u);
    const auto b = runGen(42u);
    TEST_ASSERT_EQUAL(a.chunk_ends.size(), b.chunk_ends.size());
    TEST_ASSERT_EQUAL(a.island_x.size(), b.island_x.size());
    TEST_ASSERT_EQUAL(a.coin_x.size(), b.coin_x.size());
    for (std::size_t i = 0; i < a.chunk_ends.size(); ++i) {
        TEST_ASSERT_TRUE(a.chunk_ends[i] == b.chunk_ends[i]);
    }
    for (std::size_t i = 0; i < a.island_x.size(); ++i) {
        TEST_ASSERT_TRUE(a.island_x[i] == b.island_x[i]);
        TEST_ASSERT_TRUE(a.island_y[i] == b.island_y[i]);
    }
    for (std::size_t i = 0; i < a.coin_x.size(); ++i) {
        TEST_ASSERT_TRUE(a.coin_x[i] == b.coin_x[i]);
    }
}

// AT-14: Fuzz test 1000 seeds x 3000 m
void test_AT14_generator_fuzz_1000_seeds() {
    const float max_gap_no_flap_0 = world::SolvabilityChecker::maxGapPx(0.0f, false);
    const float max_gap_flap_0 = world::SolvabilityChecker::maxGapPx(0.0f, true);
    TEST_ASSERT_TRUE(max_gap_no_flap_0 >= 32.0f);
    TEST_ASSERT_TRUE(max_gap_flap_0 >= 72.0f);

    uint64_t total_coins_all_seeds = 0;

    for (uint32_t seed = 1; seed <= 1000; ++seed) {
        world::WorldState world;
        world::MapGenerator gen;
        gen.reset(seed, world);

        world::ChunkType prev_type = world::ChunkType::Flat;
        uint8_t consecutive = 0;
        uint32_t coins_this_seed = 0;
        std::size_t seen_coin_idx = 0;

        while (gen.cursorX() < 24024.0) {
            const double start_cursor = gen.cursorX();
            const auto ch = gen.generateNextChunk(world);

            // (d) <= 2 identical chunks in a row
            if (ch.type == prev_type) {
                ++consecutive;
            } else {
                prev_type = ch.type;
                consecutive = 1;
            }
            TEST_ASSERT_TRUE(consecutive <= 2);

            // (e) Stage correctness
            const double meters = std::max(0.0, (start_cursor - config::K_START_X) / 8.0);
            if (meters < 100.0) {
                TEST_ASSERT_EQUAL(1, ch.stage);
                TEST_ASSERT_TRUE(config::K_CHUNK_WEIGHTS_S1[static_cast<std::size_t>(ch.type)] > 0);
            } else if (meters < 300.0) {
                TEST_ASSERT_EQUAL(2, ch.stage);
                TEST_ASSERT_TRUE(config::K_CHUNK_WEIGHTS_S2[static_cast<std::size_t>(ch.type)] > 0);
            } else {
                TEST_ASSERT_EQUAL(3, ch.stage);
                TEST_ASSERT_TRUE(config::K_CHUNK_WEIGHTS_S3[static_cast<std::size_t>(ch.type)] > 0);
            }

            // (b) Gap solvability limits
            if (ch.gap_px > 0.0f) {
                const float limit = world::SolvabilityChecker::maxGapPx(ch.delta_y, ch.gap_requires_flap);
                TEST_ASSERT_TRUE(ch.gap_px <= limit);
            }

            // (c) Landing zone >= 24 px after every challenge chunk
            if (ch.type != world::ChunkType::Flat) {
                TEST_ASSERT_TRUE(ch.landing_zone_w >= config::K_LANDING_ZONE_W);
            }

            // (a) No overlaps between islands; no overlaps between obstacles
            for (std::size_t i = 1; i < world.islands.size(); ++i) {
                const auto& a_isl = world.islands[i - 1];
                const auto& b_isl = world.islands[i];
                TEST_ASSERT_TRUE(a_isl.x + static_cast<double>(a_isl.w) <= b_isl.x + 0.001);
                TEST_ASSERT_TRUE(a_isl.w >= 24.0f && a_isl.w <= 120.0f);
            }
            for (std::size_t i = 1; i < world.obstacles.size(); ++i) {
                const auto& oa = world.obstacles[i - 1];
                const auto& ob = world.obstacles[i];
                TEST_ASSERT_TRUE(oa.x + static_cast<double>(oa.w) <= ob.x + 0.001);
            }

            // (g) Count coins and check vertical bounds
            for (std::size_t i = seen_coin_idx; i < world.coins.size(); ++i) {
                TEST_ASSERT_TRUE(world.coins[i].y >= 16.0f && world.coins[i].y <= config::K_VOID_Y - 24.0f);
                ++coins_this_seed;
            }
            seen_coin_idx = world.coins.size();

            // Prune well behind cursor while preserving seen_coin_idx accounting
            const std::size_t before_coins = world.coins.size();
            gen.pruneBefore(world, gen.cursorX() - 300.0);
            const std::size_t popped_coins = before_coins - world.coins.size();
            seen_coin_idx -= popped_coins;
        }

        total_coins_all_seeds += coins_this_seed;
    }

    // (f) Mean coin density: 1 coin per 8..12 m => 250..375 coins per 3000 m
    const double mean_coins_per_3000m = static_cast<double>(total_coins_all_seeds) / 1000.0;
    const double meters_per_coin = 3000.0 / mean_coins_per_3000m;
    TEST_ASSERT_TRUE_MESSAGE(meters_per_coin >= 8.0 && meters_per_coin <= 12.0,
                             "Mean coin density outside 1 coin / 8..12 m target");
}

// AT-15: 100 km simulation with pruning keeps ring buffer sizes bounded
void test_AT15_generator_100km_memory_bounded() {
    world::WorldState world;
    world::MapGenerator gen;
    gen.reset(999u, world);

    double cam_x = 0.0;
    while (cam_x < 800000.0) { // 100 km = 800,000 px
        gen.ensureGeneratedUpTo(world, cam_x + 155.0 + config::K_LOOKAHEAD_PX);
        gen.pruneBefore(world, cam_x - config::K_PRUNE_BEHIND_PX);

        TEST_ASSERT_TRUE(world.islands.size() < world.islands.capacity());
        TEST_ASSERT_TRUE(world.bridges.size() < world.bridges.capacity());
        TEST_ASSERT_TRUE(world.obstacles.size() < world.obstacles.capacity());
        TEST_ASSERT_TRUE(world.coins.size() < world.coins.capacity());
        TEST_ASSERT_TRUE(world.decors.size() < world.decors.capacity());
        TEST_ASSERT_TRUE(world.biome_markers.size() < world.biome_markers.capacity());

        cam_x += 120.0;
    }
}

void test_FR_OB_varied_obstacles_and_solvability() {
    TEST_ASSERT_TRUE(world::SolvabilityChecker::canWalkUnder(config::K_OVERHANG_CLEAR_PX));
    TEST_ASSERT_TRUE(world::SolvabilityChecker::canClearObstacle(16.0f, true));
    TEST_ASSERT_TRUE(world::SolvabilityChecker::canClearObstacle(24.0f, true));

    world::WorldState world;
    world::MapGenerator gen;
    gen.reset(77u, world);

    bool seen_overhang = false;
    bool seen_wide = false;
    bool seen_pillar = false;
    bool seen_stair = false;
    bool seen_combo_uo = false;

    while (gen.cursorX() < 32000.0) {
        const auto ch = gen.generateNextChunk(world);
        if (ch.type == world::ChunkType::ObstStair) seen_stair = true;
        if (ch.type == world::ChunkType::ComboUO) seen_combo_uo = true;

        for (std::size_t i = 0; i < world.obstacles.size(); ++i) {
            const auto& obs = world.obstacles[i];
            if (obs.type == world::ObstacleType::Overhang) {
                seen_overhang = true;
                TEST_ASSERT_TRUE(world::SolvabilityChecker::canWalkUnder(config::K_OVERHANG_CLEAR_PX));
            } else {
                if (obs.type == world::ObstacleType::WideBarrier) seen_wide = true;
                if (obs.type == world::ObstacleType::Pillar) seen_pillar = true;
                TEST_ASSERT_TRUE(world::SolvabilityChecker::canClearObstacle(obs.h, true));
            }
        }
        gen.pruneBefore(world, gen.cursorX() - 300.0);
    }

    TEST_ASSERT_TRUE(seen_overhang);
    TEST_ASSERT_TRUE(seen_wide);
    TEST_ASSERT_TRUE(seen_pillar);
    TEST_ASSERT_TRUE(seen_stair);
    TEST_ASSERT_TRUE(seen_combo_uo);
}

void test_FR_BM_biomes_cycle_without_repeats() {
    world::WorldState world;
    world::MapGenerator gen;
    gen.reset(1234u, world);

    // Starts in Greenland
    TEST_ASSERT_EQUAL(static_cast<uint8_t>(world::BiomeId::Greenland),
                      static_cast<uint8_t>(world.islands[0].biome));

    bool seen[config::K_REG_BIOME_COUNT] = {true, false, false, false, false, false};
    world::BiomeId prev = world::BiomeId::Greenland;
    int transitions = 0;
    double prev_marker_x = 0.0;

    while (gen.cursorX() < 75000.0) {
        gen.generateNextChunk(world);
        for (std::size_t i = 0; i < world.biome_markers.size(); ++i) {
            const auto& m = world.biome_markers[i];
            if (m.world_x > prev_marker_x) {
                TEST_ASSERT_TRUE(m.biome != prev);
                // Candy biome must NEVER occur from normal world biome rotation
                TEST_ASSERT_TRUE(m.biome != world::BiomeId::Candy);
                TEST_ASSERT_TRUE(static_cast<uint8_t>(m.biome) < config::K_REG_BIOME_COUNT);
                const double dist_m = (m.world_x - (prev_marker_x > 0.0 ? prev_marker_x : config::K_START_X)) / 8.0;
                TEST_ASSERT_TRUE(dist_m >= static_cast<double>(config::K_BIOME_MIN_M));
                seen[static_cast<std::size_t>(m.biome)] = true;
                prev = m.biome;
                prev_marker_x = m.world_x;
                ++transitions;
            }
        }
        gen.pruneBefore(world, gen.cursorX() - 300.0);
    }

    TEST_ASSERT_TRUE(transitions >= 8);
    for (std::size_t b = 0; b < config::K_REG_BIOME_COUNT; ++b) {
        TEST_ASSERT_TRUE(seen[b]);
    }
}

void test_FR_DC_decorators_and_perk_spawn() {
    world::WorldState world;
    world::MapGenerator gen;
    gen.reset(5678u, world);

    bool seen_cactus_small = false;
    bool seen_cactus_tall  = false;
    bool seen_jungle_tree  = false;
    bool seen_temple       = false;
    bool seen_snow_spruce  = false;
    int perk_count = 0;
    double last_perk_x = -1.0;

    while (gen.cursorX() < 70000.0) {
        gen.generateNextChunk(world);
        for (std::size_t i = 0; i < world.decors.size(); ++i) {
            const auto& d = world.decors[i];
            if (d.kind == world::DecorKind::CactusSmall) seen_cactus_small = true;
            if (d.kind == world::DecorKind::CactusTall) seen_cactus_tall = true;
            if (d.kind == world::DecorKind::JungleTree) seen_jungle_tree = true;
            if (d.kind == world::DecorKind::JungleTemple) seen_temple = true;
            if (d.kind == world::DecorKind::SnowSpruce) seen_snow_spruce = true;
        }
        for (std::size_t i = 0; i < world.perks.size(); ++i) {
            const auto& pk = world.perks[i];
            if (pk.x > last_perk_x + 0.01) {
                const double meters_from_start = (pk.x - config::K_START_X) / 8.0;
                TEST_ASSERT_TRUE(meters_from_start >= static_cast<double>(config::K_PERK_FIRST_MIN_M));
                if (last_perk_x > 0.0) {
                    const double gap_m = (pk.x - last_perk_x) / 8.0;
                    TEST_ASSERT_TRUE(gap_m >= static_cast<double>(config::K_PERK_COOLDOWN_M) - 10.0);
                }
                TEST_ASSERT_TRUE(pk.y >= config::K_PERK_CEILING_Y);
                last_perk_x = pk.x;
                ++perk_count;
            }
        }
        TEST_ASSERT_TRUE(world.decors.size() <= 96u);
        TEST_ASSERT_TRUE(world.perks.size() <= 8u);
        gen.pruneBefore(world, gen.cursorX() - 300.0);
    }

    TEST_ASSERT_TRUE(seen_cactus_small);
    TEST_ASSERT_TRUE(seen_cactus_tall);
    TEST_ASSERT_TRUE(seen_jungle_tree);
    TEST_ASSERT_TRUE(seen_temple);
    TEST_ASSERT_TRUE(seen_snow_spruce);
    TEST_ASSERT_TRUE(perk_count >= 3);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_AT13_generator_determinism);
    RUN_TEST(test_AT14_generator_fuzz_1000_seeds);
    RUN_TEST(test_AT15_generator_100km_memory_bounded);
    RUN_TEST(test_FR_OB_varied_obstacles_and_solvability);
    RUN_TEST(test_FR_BM_biomes_cycle_without_repeats);
    RUN_TEST(test_FR_DC_decorators_and_perk_spawn);
    return UNITY_END();
}
