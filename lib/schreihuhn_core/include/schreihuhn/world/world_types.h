#pragma once

#include <cstdint>
#include "schreihuhn/core/ring_buffer.h"

namespace schreihuhn::world {

enum class BiomeId : uint8_t {
    Greenland = 0,
    Taiga     = 1,
    Snow      = 2,
    Lava      = 3,
    Jungle    = 4,
    Sand      = 5,
    Candy     = 6, // Perk-only special biome
    Count     = 7
};

enum class ObstacleType : uint8_t {
    HayBale     = 0, // 8x8 ground, Stage 1+
    Fence       = 1, // 8x16 ground, Stage 2+
    Stone       = 2, // 16x16 ground, Stage 2+
    Tower       = 3, // 8x24 ground, Stage 3+
    Overhang    = 4, // 24x12 floating (+20 px clearance), Stage 1+
    WideBarrier = 5, // 24x16 ground (jump+flap / land on top), Stage 2+
    Pillar      = 6  // 16x24 ground monolith, Stage 3+
};

enum class ChunkType : uint8_t {
    Flat      = 0,
    GapS      = 1,
    GapM      = 2,
    GapL      = 3,
    ObstLow   = 4,
    ObstHigh  = 5,
    ObstTower = 6,
    BridgeS   = 7,
    BridgeL   = 8,
    Step      = 9,
    ComboOG   = 10,
    ComboBO   = 11,
    ObstUnder = 12,
    ObstWide  = 13,
    ObstStair = 14,
    ComboUO   = 15,
    Count     = 16
};

struct Island {
    double  x     = 0.0;
    float   w     = 0.0f;
    float   y     = 0.0f;
    BiomeId biome = BiomeId::Greenland;
};

struct BridgeSegment {
    double  x         = 0.0;
    float   y         = 0.0f;
    float   stability = 0.0f;
    bool    intact    = true;
    float   fall_vy   = 0.0f;
    BiomeId biome     = BiomeId::Greenland;
};

struct Obstacle {
    double  x              = 0.0;
    float   y              = 0.0f;
    float   w              = 0.0f;
    float   h              = 0.0f;
    ObstacleType type      = ObstacleType::HayBale;
    int32_t bridge_seg_idx = -1; // >= 0 if mounted on a bridge segment (optional link via x)
    bool    falling        = false;
    float   fall_vy        = 0.0f;
    BiomeId biome          = BiomeId::Greenland;
};

enum class DecorKind : uint8_t {
    GreenTree     = 0,
    GrassTuft     = 1,
    Flower        = 2,
    Spruce        = 3,
    Birch         = 4,
    Rock          = 5,
    SnowSpruce    = 6,
    LavaSpire     = 7,
    LavaCrystal   = 8,
    JungleTree    = 9,
    JungleTemple  = 10,
    Fern          = 11,
    CactusSmall   = 12,
    CactusTall    = 13,
    DesertRock    = 14,
    CandyLollipop = 15,
    CandyCane     = 16
};

struct DecorItem {
    double    x     = 0.0;
    float     y     = 0.0f; // base sits on island top y
    DecorKind kind  = DecorKind::GrassTuft;
    BiomeId   biome = BiomeId::Greenland;
};

struct PerkItem {
    double x         = 0.0;
    float  y         = 0.0f;
    bool   collected = false;
};

struct BiomeMarker {
    double  world_x = 0.0;
    BiomeId biome   = BiomeId::Greenland;
};

struct AmbientParticle {
    float   x     = 0.0f; // screen-space x [0..155)
    float   y     = 0.0f; // screen-space y [0..155)
    float   vx    = 0.0f; // px/s
    float   vy    = 0.0f; // px/s
    float   phase = 0.0f; // wobble / trail animation phase
    uint8_t kind  = 0;    // 0=snow, 1=comet, 2=ember, 3=rain, 4=candy star
    BiomeId biome = BiomeId::Snow;
};

struct Coin {
    double x         = 0.0;
    float  y         = 0.0f;
    bool   collected = false;
};

struct Particle {
    double  x        = 0.0;
    float   y        = 0.0f;
    float   vx       = 0.0f;
    float   vy       = 0.0f;
    float   age      = 0.0f;
    float   lifetime = 0.18f;
    uint8_t kind     = 0; // 0 = coin sparkle, 1 = feather, 2 = dust
};

struct WorldState {
    core::StaticRingBuffer<Island, 32>         islands;
    core::StaticRingBuffer<BridgeSegment, 64>  bridges;
    core::StaticRingBuffer<Obstacle, 32>       obstacles;
    core::StaticRingBuffer<Coin, 128>          coins;
    core::StaticRingBuffer<Particle, 32>       particles;
    core::StaticRingBuffer<DecorItem, 96>      decors;
    core::StaticRingBuffer<PerkItem, 8>        perks;
    core::StaticRingBuffer<BiomeMarker, 16>    biome_markers;

    void clear() {
        islands.clear();
        bridges.clear();
        obstacles.clear();
        coins.clear();
        particles.clear();
        decors.clear();
        perks.clear();
        biome_markers.clear();
    }
};

} // namespace schreihuhn::world
