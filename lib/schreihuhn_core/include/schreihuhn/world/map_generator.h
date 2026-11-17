#pragma once

#include <array>
#include <cstdint>
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/world/chunk_decorator.h"
#include "schreihuhn/world/world_types.h"

namespace schreihuhn::world {

struct GeneratedChunkInfo {
    ChunkType type          = ChunkType::Flat;
    uint8_t   stage         = 1;
    double    start_x       = 0.0;
    double    end_x         = 0.0;
    float     gap_px        = 0.0f;
    bool      gap_requires_flap = false;
    float     delta_y       = 0.0f;
    float     landing_zone_w = 0.0f;
};

class MapGenerator {
public:
    void reset(uint32_t seed, WorldState& world);

    GeneratedChunkInfo generateNextChunk(WorldState& world);
    void ensureGeneratedUpTo(WorldState& world, double target_x);
    void pruneBefore(WorldState& world, double min_x);

    double cursorX() const { return cursor_x_; }
    float  currentLevelY() const { return current_level_y_; }
    uint32_t seed() const { return seed_; }

private:
    uint8_t stageAtCursor() const;
    ChunkType pickChunkType(uint8_t stage);
    void maybeSpawnCoinRow(WorldState& world, double start_x, float width, float ground_y,
                           uint8_t count, float height_above_ground = 15.0f);
    void maybeSpawnCoinArc(WorldState& world, double jump_x, float start_y, float delta_y,
                           float span_x, bool with_flap, uint8_t count);
    void addIsland(WorldState& world, double x, float w, float y);
    void addBridge(WorldState& world, double x, uint8_t segs, float y);
    void addObstacle(WorldState& world, double x, float ground_y, ObstacleType type);
    void advanceBiomeIfReady(WorldState& world);
    void rollNextBiomeSwitchX(double from_x);

    uint32_t seed_ = 1;
    core::XorShift32 rng_{1};
    double cursor_x_ = 0.0;
    float  current_level_y_ = 104.0f;
    ChunkType last_chunk_ = ChunkType::Flat;
    uint8_t repeat_count_ = 0;
    uint32_t coin_cooldown_ = 0;
    BiomeId gen_biome_ = BiomeId::Greenland;
    double next_biome_switch_x_ = 0.0;

    BiomePropDecorator prop_decorator_{};
    PerkSpawnDecorator perk_decorator_{};
    std::array<IChunkDecorator*, 2> decorators_{&prop_decorator_, &perk_decorator_};
};

} // namespace schreihuhn::world
