#pragma once

#include <array>
#include <cstdint>
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/world/world_types.h"

namespace schreihuhn::world {

struct ChunkDecorIsland {
    double x        = 0.0;
    float  w        = 0.0f;
    float  ground_y = 104.0f;
    double avoid_x0 = 0.0;
    double avoid_x1 = -1.0; // avoid_x1 <= avoid_x0 means no exclusion zone
};

struct ChunkContext {
    ChunkType type  = ChunkType::Flat;
    BiomeId   biome = BiomeId::Greenland;
    double    start_x = 0.0;
    double    end_x   = 0.0;
    std::array<ChunkDecorIsland, 2> islands{};
    uint8_t   island_count = 0;

    void addIslandSpan(double x, float w, float ground_y,
                       double avoid_x0 = 0.0, double avoid_x1 = -1.0) {
        if (island_count < islands.size()) {
            islands[island_count++] = ChunkDecorIsland{x, w, ground_y, avoid_x0, avoid_x1};
        }
    }
};

// Polymorphic Decorator interface applied to generated base chunks without heap allocation.
class IChunkDecorator {
public:
    virtual ~IChunkDecorator() = default;
    virtual void reset() = 0;
    virtual void decorate(const ChunkContext& ctx,
                          WorldState& world,
                          core::XorShift32& rng) = 0;
};

// Decorates islands with biome-specific non-colliding props (trees, temples, cacti, etc.).
class BiomePropDecorator : public IChunkDecorator {
public:
    void reset() override {}
    void decorate(const ChunkContext& ctx,
                  WorldState& world,
                  core::XorShift32& rng) override;
};

// Probabilistically inserts the white-packet flight perk on safe walkable stretches.
class PerkSpawnDecorator : public IChunkDecorator {
public:
    void reset() override { last_perk_x_ = 0.0; }
    void decorate(const ChunkContext& ctx,
                  WorldState& world,
                  core::XorShift32& rng) override;

private:
    double last_perk_x_ = 0.0;
};

} // namespace schreihuhn::world
