#include "schreihuhn/world/chunk_decorator.h"
#include "schreihuhn/config/tuning.h"

#include <algorithm>

namespace schreihuhn::world {

namespace {

DecorKind pickPropForBiome(BiomeId biome, core::XorShift32& rng) {
    const int roll = rng.rangeInt(0, 99);
    switch (biome) {
        case BiomeId::Greenland:
            if (roll < 35) return DecorKind::GreenTree;
            if (roll < 75) return DecorKind::GrassTuft;
            return DecorKind::Flower;
        case BiomeId::Taiga:
            if (roll < 42) return DecorKind::Spruce;
            if (roll < 72) return DecorKind::Birch;
            return DecorKind::Rock;
        case BiomeId::Snow:
            if (roll < 65) return DecorKind::SnowSpruce;
            return DecorKind::Rock;
        case BiomeId::Lava:
            if (roll < 55) return DecorKind::LavaSpire;
            return DecorKind::LavaCrystal;
        case BiomeId::Jungle:
            if (roll < 40) return DecorKind::JungleTree;
            if (roll < 62) return DecorKind::JungleTemple;
            return DecorKind::Fern;
        case BiomeId::Sand:
            if (roll < 46) return DecorKind::CactusTall;
            if (roll < 82) return DecorKind::CactusSmall;
            return DecorKind::DesertRock;
        case BiomeId::Candy:
            return (roll < 55) ? DecorKind::CandyLollipop : DecorKind::CandyCane;
        default:
            return DecorKind::GrassTuft;
    }
}

void propDensityForBiome(BiomeId biome, int& out_prob, int& out_step_min, int& out_step_max) {
    switch (biome) {
        case BiomeId::Sand:
            out_prob = 80; out_step_min = 14; out_step_max = 22;
            break;
        case BiomeId::Jungle:
            out_prob = 76; out_step_min = 16; out_step_max = 24;
            break;
        case BiomeId::Taiga:
            out_prob = 68; out_step_min = 18; out_step_max = 28;
            break;
        case BiomeId::Snow:
            out_prob = 65; out_step_min = 20; out_step_max = 30;
            break;
        case BiomeId::Lava:
            out_prob = 60; out_step_min = 20; out_step_max = 30;
            break;
        default:
            out_prob = 62; out_step_min = 22; out_step_max = 32;
            break;
    }
}

} // namespace

void BiomePropDecorator::decorate(const ChunkContext& ctx,
                                  WorldState& world,
                                  core::XorShift32& rng) {
    int spawn_prob = 62;
    int step_min = 22;
    int step_max = 32;
    propDensityForBiome(ctx.biome, spawn_prob, step_min, step_max);

    for (uint8_t idx = 0; idx < ctx.island_count; ++idx) {
        const auto& isl = ctx.islands[idx];
        if (isl.w < 24.0f || world.decors.full()) {
            continue;
        }
        double pos = isl.x + 5.0;
        const double end_limit = isl.x + static_cast<double>(isl.w) - 12.0;
        while (pos <= end_limit && !world.decors.full()) {
            const bool blocked = (isl.avoid_x1 > isl.avoid_x0) &&
                                 (pos + 12.0 > isl.avoid_x0 && pos < isl.avoid_x1);
            if (!blocked && rng.rangeInt(0, 99) < spawn_prob) {
                world.decors.pushBack(DecorItem{
                    pos, isl.ground_y, pickPropForBiome(ctx.biome, rng), ctx.biome});
                pos += static_cast<double>(rng.rangeInt(step_min, step_max));
            } else {
                pos += 12.0;
            }
        }
    }
}

void PerkSpawnDecorator::decorate(const ChunkContext& ctx,
                                  WorldState& world,
                                  core::XorShift32& rng) {
    if (ctx.island_count == 0 || world.perks.full()) {
        return;
    }
    const double meters_from_start = std::max(0.0, (ctx.start_x - config::K_START_X) / 8.0);
    if (meters_from_start < static_cast<double>(config::K_PERK_FIRST_MIN_M)) {
        return;
    }
    const double dist_since_last_m = (ctx.start_x - last_perk_x_) / 8.0;
    if (last_perk_x_ > 0.0 && dist_since_last_m < static_cast<double>(config::K_PERK_COOLDOWN_M)) {
        return;
    }
    if (rng.rangeInt(0, 99) >= 42) {
        return;
    }

    // Prefer the trailing / landing island of the chunk for safe, unblocked pickup placement
    const auto& isl = ctx.islands[ctx.island_count - 1u];
    if (isl.w < 24.0f) {
        return;
    }
    double spawn_x = isl.x + static_cast<double>(isl.w) * 0.5 - 4.0;
    const bool has_avoid = (isl.avoid_x1 > isl.avoid_x0);
    if (has_avoid) {
        // Place cleanly in the trailing landing zone past obstacles if roomy enough
        const double zone_start = std::max(isl.x + 4.0, isl.avoid_x1 + 4.0);
        const double zone_end   = isl.x + static_cast<double>(isl.w) - 10.0;
        if (zone_end - zone_start < 6.0) {
            return;
        }
        spawn_x = 0.5 * (zone_start + zone_end);
    }

    const float spawn_y = core::clampf(isl.ground_y - 16.0f,
                                       config::K_PERK_CEILING_Y + 4.0f,
                                       config::K_VOID_Y - 26.0f);
    world.perks.pushBack(PerkItem{spawn_x, spawn_y, false});
    last_perk_x_ = spawn_x;
}

} // namespace schreihuhn::world
