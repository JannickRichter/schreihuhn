#include "schreihuhn/world/map_generator.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/world/solvability.h"

#include <algorithm>
#include <cmath>

namespace schreihuhn::world {

namespace {

float snapTile(float px) {
    return std::round(px / config::K_TILE_SIZE) * config::K_TILE_SIZE;
}

void obstacleDims(ObstacleType type, float& out_w, float& out_h) {
    switch (type) {
        case ObstacleType::HayBale:     out_w = 8.0f;  out_h = 8.0f;  break;
        case ObstacleType::Fence:       out_w = 8.0f;  out_h = 16.0f; break;
        case ObstacleType::Stone:       out_w = 16.0f; out_h = 16.0f; break;
        case ObstacleType::Tower:       out_w = 8.0f;  out_h = 24.0f; break;
        case ObstacleType::Overhang:    out_w = 24.0f; out_h = 12.0f; break;
        case ObstacleType::WideBarrier: out_w = 24.0f; out_h = 16.0f; break;
        case ObstacleType::Pillar:      out_w = 16.0f; out_h = 24.0f; break;
        default:                        out_w = 8.0f;  out_h = 8.0f;  break;
    }
}

} // namespace

void MapGenerator::reset(uint32_t seed, WorldState& world) {
    seed_ = (seed == 0u) ? 1u : seed;
    rng_.reseed(seed_);
    world.clear();
    for (IChunkDecorator* dec : decorators_) {
        dec->reset();
    }

    gen_biome_ = BiomeId::Greenland;
    rollNextBiomeSwitchX(config::K_START_X);

    // FR-MP-03: Start island 96 px wide at K_LEVEL_0, no obstacles
    addIsland(world, 0.0, config::K_START_ISLAND_W, config::K_LEVEL_0);
    ChunkContext start_ctx{};
    start_ctx.type = ChunkType::Flat;
    start_ctx.biome = gen_biome_;
    start_ctx.start_x = 0.0;
    start_ctx.end_x = static_cast<double>(config::K_START_ISLAND_W);
    start_ctx.addIslandSpan(0.0, config::K_START_ISLAND_W, config::K_LEVEL_0, 14.0, 38.0);
    prop_decorator_.decorate(start_ctx, world, rng_);

    cursor_x_        = static_cast<double>(config::K_START_ISLAND_W);
    current_level_y_ = config::K_LEVEL_0;
    last_chunk_      = ChunkType::Flat;
    repeat_count_    = 0;
    coin_cooldown_   = 1;
}

void MapGenerator::rollNextBiomeSwitchX(double from_x) {
    const int dist_m = rng_.rangeInt(static_cast<int>(config::K_BIOME_MIN_M),
                                     static_cast<int>(config::K_BIOME_MAX_M));
    next_biome_switch_x_ = from_x + static_cast<double>(dist_m) * 8.0;
}

void MapGenerator::advanceBiomeIfReady(WorldState& world) {
    if (cursor_x_ < next_biome_switch_x_) {
        return;
    }
    const uint8_t cur_idx = static_cast<uint8_t>(gen_biome_);
    const uint8_t count = config::K_REG_BIOME_COUNT; // Excludes Candy (Perk-only)
    const uint8_t offset = static_cast<uint8_t>(rng_.rangeInt(1, static_cast<int>(count) - 1));
    gen_biome_ = static_cast<BiomeId>((cur_idx + offset) % count);
    if (world.biome_markers.full()) {
        world.biome_markers.popFront();
    }
    world.biome_markers.pushBack(BiomeMarker{cursor_x_, gen_biome_});
    rollNextBiomeSwitchX(cursor_x_);
}

uint8_t MapGenerator::stageAtCursor() const {
    const double meters = std::max(0.0, (cursor_x_ - config::K_START_X) / 8.0);
    if (meters < static_cast<double>(config::K_STAGE2_SCORE)) {
        return 1;
    }
    if (meters < static_cast<double>(config::K_STAGE3_SCORE)) {
        return 2;
    }
    return 3;
}

ChunkType MapGenerator::pickChunkType(uint8_t stage) {
    const auto& weights = (stage == 1)
                              ? config::K_CHUNK_WEIGHTS_S1
                              : (stage == 2 ? config::K_CHUNK_WEIGHTS_S2
                                            : config::K_CHUNK_WEIGHTS_S3);

    uint32_t total = 0;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const ChunkType ct = static_cast<ChunkType>(i);
        if (ct == last_chunk_ && repeat_count_ >= 2) {
            continue;
        }
        total += weights[i];
    }

    if (total == 0) {
        return ChunkType::Flat;
    }

    uint32_t roll = rng_.nextU32() % total;
    for (std::size_t i = 0; i < weights.size(); ++i) {
        const ChunkType ct = static_cast<ChunkType>(i);
        if (ct == last_chunk_ && repeat_count_ >= 2) {
            continue;
        }
        if (roll < weights[i]) {
            return ct;
        }
        roll -= weights[i];
    }
    return ChunkType::Flat;
}

void MapGenerator::addIsland(WorldState& world, double x, float w, float y) {
    world.islands.pushBack(Island{x, w, y, gen_biome_});
}

void MapGenerator::addBridge(WorldState& world, double x, uint8_t segs, float y) {
    for (uint8_t i = 0; i < segs; ++i) {
        BridgeSegment seg{};
        seg.x         = x + static_cast<double>(i) * config::K_BRIDGE_SEG_W;
        seg.y         = y;
        seg.stability = config::K_BRIDGE_STABILITY;
        seg.intact    = true;
        seg.fall_vy   = 0.0f;
        seg.biome     = gen_biome_;
        world.bridges.pushBack(seg);
    }
}

void MapGenerator::addObstacle(WorldState& world, double x, float ground_y, ObstacleType type) {
    float w = 8.0f;
    float h = 8.0f;
    obstacleDims(type, w, h);
    Obstacle obs{};
    obs.x       = x;
    obs.y       = (type == ObstacleType::Overhang)
                      ? (ground_y - config::K_OVERHANG_CLEAR_PX - h)
                      : (ground_y - h);
    obs.w       = w;
    obs.h       = h;
    obs.type    = type;
    obs.biome   = gen_biome_;
    obs.falling = false;
    obs.fall_vy = 0.0f;
    world.obstacles.pushBack(obs);
}

void MapGenerator::maybeSpawnCoinRow(WorldState& world, double start_x, float width,
                                     float ground_y, uint8_t count, float height_above_ground) {
    if (coin_cooldown_ > 0) {
        --coin_cooldown_;
        return;
    }
    if (rng_.rangeInt(0, 99) >= 52) {
        return;
    }
    const float span = static_cast<float>(count - 1) * 12.0f + 8.0f;
    if (width < span + 8.0f) {
        return;
    }
    const double base_x = start_x + static_cast<double>((width - span) * 0.5f);
    const float coin_y  = core::clampf(ground_y - height_above_ground, 16.0f, config::K_VOID_Y - 24.0f);
    for (uint8_t i = 0; i < count; ++i) {
        world.coins.pushBack(Coin{base_x + static_cast<double>(i) * 12.0, coin_y, false});
    }
    coin_cooldown_ = 1;
}

void MapGenerator::maybeSpawnCoinArc(WorldState& world, double jump_x, float start_y,
                                     float delta_y, float span_x, bool with_flap, uint8_t count) {
    if (coin_cooldown_ > 0) {
        --coin_cooldown_;
        return;
    }
    if (rng_.rangeInt(0, 99) >= 58) {
        return;
    }
    const JumpSimResult sim = SolvabilityChecker::simulateJump(delta_y, with_flap);
    if (sim.sample_count < 4 || sim.distance_px <= 1.0f) {
        return;
    }
    const float scale = (span_x > 0.0f) ? std::min(1.0f, span_x / sim.distance_px) : 1.0f;
    for (uint8_t i = 0; i < count; ++i) {
        const float frac = 0.22f + (0.56f * static_cast<float>(i)) /
                                       static_cast<float>(std::max<uint8_t>(1, count - 1));
        const std::size_t idx = std::min(
            sim.sample_count - 1,
            static_cast<std::size_t>(frac * static_cast<float>(sim.sample_count - 1)));
        const float sx = sim.samples[idx].x * scale;
        const float sy = sim.samples[idx].y;
        const float cy = core::clampf(start_y + sy - 10.0f, 16.0f, config::K_VOID_Y - 24.0f);
        world.coins.pushBack(Coin{jump_x + static_cast<double>(sx) - 4.0, cy, false});
    }
    coin_cooldown_ = 1;
}

GeneratedChunkInfo MapGenerator::generateNextChunk(WorldState& world) {
    advanceBiomeIfReady(world);

    const uint8_t stage = stageAtCursor();
    const ChunkType ct = pickChunkType(stage);

    if (ct == last_chunk_) {
        ++repeat_count_;
    } else {
        last_chunk_ = ct;
        repeat_count_ = 1;
    }

    GeneratedChunkInfo info{};
    info.type    = ct;
    info.stage   = stage;
    info.start_x = cursor_x_;

    ChunkContext ctx{};
    ctx.type    = ct;
    ctx.biome   = gen_biome_;
    ctx.start_x = cursor_x_;

    switch (ct) {
        case ChunkType::Flat: {
            const float w = static_cast<float>(rng_.rangeInt(5, 12)) * config::K_TILE_SIZE; // 40..96
            addIsland(world, cursor_x_, w, current_level_y_);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_);
            const uint8_t cnt = static_cast<uint8_t>(rng_.rangeInt(3, 5));
            maybeSpawnCoinRow(world, cursor_x_, w, current_level_y_, cnt);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = w;
            break;
        }

        case ChunkType::GapS: {
            const float max_g = SolvabilityChecker::maxGapPx(0.0f, false);
            float gap = static_cast<float>(rng_.rangeInt(2, 3)) * config::K_TILE_SIZE; // 16..24
            gap = std::min(gap, max_g);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 6)) * config::K_TILE_SIZE; // 24..48
            const double gap_start = cursor_x_;
            addIsland(world, cursor_x_ + gap, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + gap, land_w, current_level_y_);
            maybeSpawnCoinArc(world, gap_start - 4.0, current_level_y_, 0.0f, gap + 8.0f, false, 3);
            cursor_x_ += static_cast<double>(gap + land_w);
            info.gap_px = gap;
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::GapM: {
            const float max_g = SolvabilityChecker::maxGapPx(0.0f, false);
            float gap = static_cast<float>(rng_.rangeInt(4, 5)) * config::K_TILE_SIZE; // 32..40
            gap = std::min(gap, max_g);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 6)) * config::K_TILE_SIZE; // 24..48
            const double gap_start = cursor_x_;
            addIsland(world, cursor_x_ + gap, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + gap, land_w, current_level_y_);
            maybeSpawnCoinArc(world, gap_start - 4.0, current_level_y_, 0.0f, gap + 8.0f, false, 3);
            cursor_x_ += static_cast<double>(gap + land_w);
            info.gap_px = gap;
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::GapL: {
            const float max_g = SolvabilityChecker::maxGapPx(0.0f, true);
            float gap = static_cast<float>(rng_.rangeInt(6, 10)) * config::K_TILE_SIZE; // 48..80
            gap = std::min(gap, max_g);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 6)) * config::K_TILE_SIZE; // 24..48
            const double gap_start = cursor_x_;
            addIsland(world, cursor_x_ + gap, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + gap, land_w, current_level_y_);
            const uint8_t cnt = static_cast<uint8_t>(rng_.rangeInt(3, 4));
            maybeSpawnCoinArc(world, gap_start - 4.0, current_level_y_, 0.0f, gap + 8.0f, true, cnt);
            cursor_x_ += static_cast<double>(gap + land_w);
            info.gap_px = gap;
            info.gap_requires_flap = true;
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstLow: {
            const int num = rng_.rangeInt(1, 2);
            const float land_w = 24.0f;
            float w = 0.0f;
            if (num == 1) {
                w = 16.0f + 8.0f + land_w; // 48 px
                addIsland(world, cursor_x_, w, current_level_y_);
                addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::HayBale);
                ctx.addIslandSpan(cursor_x_, w, current_level_y_, cursor_x_ + 12.0, cursor_x_ + 28.0);
                maybeSpawnCoinArc(world, cursor_x_ + 4.0, current_level_y_, 0.0f, 32.0f, false, 3);
            } else {
                w = 16.0f + 8.0f + 24.0f + 8.0f + land_w; // 80 px
                addIsland(world, cursor_x_, w, current_level_y_);
                addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::HayBale);
                addObstacle(world, cursor_x_ + 48.0, current_level_y_, ObstacleType::HayBale);
                ctx.addIslandSpan(cursor_x_, w, current_level_y_, cursor_x_ + 12.0, cursor_x_ + 60.0);
                maybeSpawnCoinArc(world, cursor_x_ + 4.0, current_level_y_, 0.0f, 32.0f, false, 3);
            }
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstHigh: {
            const ObstacleType ot = (rng_.rangeInt(0, 1) == 0)
                                        ? ObstacleType::Fence
                                        : ObstacleType::Stone;
            float ow = 8.0f;
            float oh = 16.0f;
            obstacleDims(ot, ow, oh);
            const float land_w = 24.0f;
            const float w = 16.0f + ow + land_w; // 48 or 56 px
            addIsland(world, cursor_x_, w, current_level_y_);
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ot);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_,
                              cursor_x_ + 12.0, cursor_x_ + 20.0 + ow);
            maybeSpawnCoinArc(world, cursor_x_ + 4.0, current_level_y_, 0.0f, 32.0f + ow, false, 3);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstTower: {
            const ObstacleType ot = (rng_.rangeInt(0, 1) == 0)
                                        ? ObstacleType::Tower
                                        : ObstacleType::Pillar;
            float ow = 8.0f;
            float oh = 24.0f;
            obstacleDims(ot, ow, oh);
            const float land_w = 24.0f;
            const float w = 16.0f + ow + land_w; // 48 or 56 px
            addIsland(world, cursor_x_, w, current_level_y_);
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ot);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_,
                              cursor_x_ + 12.0, cursor_x_ + 20.0 + ow);
            maybeSpawnCoinArc(world, cursor_x_ + 4.0, current_level_y_, 0.0f, 32.0f + ow, ot == ObstacleType::Pillar, 3);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::BridgeS: {
            const uint8_t segs = static_cast<uint8_t>(rng_.rangeInt(3, 4));
            const float bw = static_cast<float>(segs) * config::K_BRIDGE_SEG_W;
            const float land_w = static_cast<float>(rng_.rangeInt(3, 5)) * config::K_TILE_SIZE;
            addBridge(world, cursor_x_, segs, current_level_y_);
            addIsland(world, cursor_x_ + bw, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + bw, land_w, current_level_y_);
            maybeSpawnCoinRow(world, cursor_x_, bw, current_level_y_, 3);
            cursor_x_ += static_cast<double>(bw + land_w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::BridgeL: {
            const uint8_t segs = static_cast<uint8_t>(rng_.rangeInt(5, 8));
            const float bw = static_cast<float>(segs) * config::K_BRIDGE_SEG_W;
            const float land_w = static_cast<float>(rng_.rangeInt(3, 6)) * config::K_TILE_SIZE;
            addBridge(world, cursor_x_, segs, current_level_y_);
            addIsland(world, cursor_x_ + bw, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + bw, land_w, current_level_y_);
            maybeSpawnCoinRow(world, cursor_x_, bw, current_level_y_, 4);
            cursor_x_ += static_cast<double>(bw + land_w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::Step: {
            const float min_y = (stage == 2) ? config::K_LEVEL_UP1 : config::K_LEVEL_UP2;
            const float max_y = (stage == 2) ? config::K_LEVEL_DOWN1 : config::K_LEVEL_DOWN2;
            const float step_mag = (stage >= 3 && rng_.rangeInt(0, 1) == 1) ? 16.0f : 8.0f;

            float delta_y = (rng_.rangeInt(0, 1) == 0) ? -step_mag : step_mag;
            float next_y = snapTile(current_level_y_ + delta_y);
            if (next_y < min_y || next_y > max_y) {
                delta_y = -delta_y;
                next_y = snapTile(current_level_y_ + delta_y);
                if (next_y < min_y || next_y > max_y) {
                    next_y = core::clampf(next_y, min_y, max_y);
                    delta_y = next_y - current_level_y_;
                }
            }
            if (std::fabs(delta_y) < 1.0f) {
                delta_y = (current_level_y_ > config::K_LEVEL_0) ? -8.0f : 8.0f;
                next_y = core::clampf(current_level_y_ + delta_y, min_y, max_y);
                delta_y = next_y - current_level_y_;
            }

            const float max_g = SolvabilityChecker::maxGapPx(delta_y, false);
            float gap = static_cast<float>(rng_.rangeInt(2, 3)) * config::K_TILE_SIZE; // 16..24
            gap = std::min(gap, max_g);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 6)) * config::K_TILE_SIZE;
            const double gap_start = cursor_x_;
            const float prev_y = current_level_y_;
            current_level_y_ = next_y;
            addIsland(world, cursor_x_ + gap, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + gap, land_w, current_level_y_);
            maybeSpawnCoinArc(world, gap_start - 4.0, prev_y, delta_y, gap + 8.0f, false, 3);
            cursor_x_ += static_cast<double>(gap + land_w);
            info.gap_px = gap;
            info.delta_y = delta_y;
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ComboOG: {
            // Obstacle followed by 16 px flat ground before GAP_S, then landing zone >= 24 px
            const float isl_w = 16.0f + 8.0f + 16.0f; // 40 px
            addIsland(world, cursor_x_, isl_w, current_level_y_);
            const ObstacleType ot = (rng_.rangeInt(0, 1) == 0)
                                        ? ObstacleType::HayBale
                                        : ObstacleType::Fence;
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ot);
            ctx.addIslandSpan(cursor_x_, isl_w, current_level_y_, cursor_x_ + 12.0, cursor_x_ + 28.0);
            const float max_g = SolvabilityChecker::maxGapPx(0.0f, false);
            float gap = static_cast<float>(rng_.rangeInt(2, 3)) * config::K_TILE_SIZE; // 16..24
            gap = std::min(gap, max_g);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 5)) * config::K_TILE_SIZE;
            addIsland(world, cursor_x_ + isl_w + gap, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + isl_w + gap, land_w, current_level_y_);
            maybeSpawnCoinArc(world, cursor_x_ + isl_w - 4.0, current_level_y_, 0.0f,
                              gap + 8.0f, false, 3);
            cursor_x_ += static_cast<double>(isl_w + gap + land_w);
            info.gap_px = gap;
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ComboBO: {
            // 5-7 segment bridge with HayBale on middle segment, then landing zone >= 24 px
            const uint8_t segs = static_cast<uint8_t>(rng_.rangeInt(5, 7));
            const float bw = static_cast<float>(segs) * config::K_BRIDGE_SEG_W;
            const uint8_t mid = static_cast<uint8_t>(segs / 2u);
            const float land_w = static_cast<float>(rng_.rangeInt(3, 5)) * config::K_TILE_SIZE;
            addBridge(world, cursor_x_, segs, current_level_y_);
            addObstacle(world, cursor_x_ + static_cast<double>(mid) * config::K_BRIDGE_SEG_W,
                        current_level_y_, ObstacleType::HayBale);
            addIsland(world, cursor_x_ + bw, land_w, current_level_y_);
            ctx.addIslandSpan(cursor_x_ + bw, land_w, current_level_y_);
            maybeSpawnCoinArc(world, cursor_x_ + static_cast<double>(mid) * config::K_BRIDGE_SEG_W - 10.0,
                              current_level_y_, 0.0f, 28.0f, false, 3);
            cursor_x_ += static_cast<double>(bw + land_w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstUnder: {
            // Walk calmly underneath Overhang (20 px ground clearance > 13 px hitbox)
            const float land_w = 24.0f;
            const float w = 16.0f + 24.0f + land_w; // 64 px
            addIsland(world, cursor_x_, w, current_level_y_);
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::Overhang);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_, cursor_x_ + 12.0, cursor_x_ + 44.0);
            maybeSpawnCoinRow(world, cursor_x_ + 12.0, 32.0f, current_level_y_, 3, 9.0f);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstWide: {
            // Wide barrier (24x16): requires Jump + Flap or intermediate top landing
            const float land_w = 24.0f;
            const float w = 16.0f + 24.0f + land_w; // 64 px
            addIsland(world, cursor_x_, w, current_level_y_);
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::WideBarrier);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_, cursor_x_ + 12.0, cursor_x_ + 44.0);
            maybeSpawnCoinArc(world, cursor_x_ + 4.0, current_level_y_, 0.0f, 48.0f, true, 3);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ObstStair: {
            // Stepped hurdle: small 8x8 step, 20 px gap, then tall Fence (S2) or Pillar/Tower (S3)
            const ObstacleType top_ot = (stage >= 3)
                                            ? ((rng_.rangeInt(0, 1) == 0) ? ObstacleType::Pillar
                                                                          : ObstacleType::Tower)
                                            : ObstacleType::Fence;
            float ow = 8.0f;
            float oh = 16.0f;
            obstacleDims(top_ot, ow, oh);
            const float land_w = 24.0f;
            const float w = 16.0f + 8.0f + 20.0f + ow + land_w; // 72 or 80 px
            addIsland(world, cursor_x_, w, current_level_y_);
            addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::HayBale);
            addObstacle(world, cursor_x_ + 44.0, current_level_y_, top_ot);
            ctx.addIslandSpan(cursor_x_, w, current_level_y_,
                              cursor_x_ + 12.0, cursor_x_ + 48.0 + ow);
            maybeSpawnCoinArc(world, cursor_x_ + 12.0, current_level_y_, 0.0f, 44.0f + ow, true, 3);
            cursor_x_ += static_cast<double>(w);
            info.landing_zone_w = land_w;
            break;
        }

        case ChunkType::ComboUO: {
            // Technique switch: calm TALK under Overhang -> immediate SHOUT over Fence or GapS
            const bool end_with_gap = (stage >= 3 && rng_.rangeInt(0, 1) == 1);
            if (!end_with_gap) {
                const float land_w = 24.0f;
                const float isl_w = 16.0f + 24.0f + 24.0f + 8.0f + land_w; // 96 px <= 120 px
                addIsland(world, cursor_x_, isl_w, current_level_y_);
                addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::Overhang);
                addObstacle(world, cursor_x_ + 64.0, current_level_y_, ObstacleType::Fence);
                ctx.addIslandSpan(cursor_x_, isl_w, current_level_y_,
                                  cursor_x_ + 12.0, cursor_x_ + 76.0);
                maybeSpawnCoinRow(world, cursor_x_ + 12.0, 32.0f, current_level_y_, 3, 9.0f);
                cursor_x_ += static_cast<double>(isl_w);
                info.landing_zone_w = land_w;
            } else {
                const float isl_w = 16.0f + 24.0f + 24.0f; // 64 px
                addIsland(world, cursor_x_, isl_w, current_level_y_);
                addObstacle(world, cursor_x_ + 16.0, current_level_y_, ObstacleType::Overhang);
                ctx.addIslandSpan(cursor_x_, isl_w, current_level_y_,
                                  cursor_x_ + 12.0, cursor_x_ + 44.0);
                const float max_g = SolvabilityChecker::maxGapPx(0.0f, false);
                float gap = static_cast<float>(rng_.rangeInt(2, 3)) * config::K_TILE_SIZE;
                gap = std::min(gap, max_g);
                const float land_w = static_cast<float>(rng_.rangeInt(3, 5)) * config::K_TILE_SIZE;
                addIsland(world, cursor_x_ + isl_w + gap, land_w, current_level_y_);
                ctx.addIslandSpan(cursor_x_ + isl_w + gap, land_w, current_level_y_);
                maybeSpawnCoinArc(world, cursor_x_ + isl_w - 4.0, current_level_y_, 0.0f,
                                  gap + 8.0f, false, 3);
                cursor_x_ += static_cast<double>(isl_w + gap + land_w);
                info.gap_px = gap;
                info.landing_zone_w = land_w;
            }
            break;
        }

        default:
            break;
    }

    ctx.end_x = cursor_x_;
    for (IChunkDecorator* dec : decorators_) {
        dec->decorate(ctx, world, rng_);
    }

    info.end_x = cursor_x_;
    return info;
}

void MapGenerator::ensureGeneratedUpTo(WorldState& world, double target_x) {
    while (cursor_x_ < target_x) {
        generateNextChunk(world);
    }
}

void MapGenerator::pruneBefore(WorldState& world, double min_x) {
    while (!world.islands.empty()) {
        const auto& isl = world.islands[0];
        if (isl.x + static_cast<double>(isl.w) < min_x) {
            world.islands.popFront();
        } else {
            break;
        }
    }
    while (!world.bridges.empty()) {
        const auto& seg = world.bridges[0];
        if (seg.x + static_cast<double>(config::K_BRIDGE_SEG_W) < min_x) {
            world.bridges.popFront();
        } else {
            break;
        }
    }
    while (!world.obstacles.empty()) {
        const auto& obs = world.obstacles[0];
        if (obs.x + static_cast<double>(obs.w) < min_x) {
            world.obstacles.popFront();
        } else {
            break;
        }
    }
    while (!world.coins.empty()) {
        const auto& c = world.coins[0];
        if (c.x + 8.0 < min_x) {
            world.coins.popFront();
        } else {
            break;
        }
    }
    while (!world.decors.empty()) {
        const auto& d = world.decors[0];
        if (d.x + 24.0 < min_x) {
            world.decors.popFront();
        } else {
            break;
        }
    }
    while (!world.perks.empty()) {
        const auto& pk = world.perks[0];
        if (pk.x + 8.0 < min_x) {
            world.perks.popFront();
        } else {
            break;
        }
    }
    while (!world.biome_markers.empty()) {
        const auto& m = world.biome_markers[0];
        if (m.world_x < min_x - 32.0) {
            world.biome_markers.popFront();
        } else {
            break;
        }
    }
}

} // namespace schreihuhn::world
