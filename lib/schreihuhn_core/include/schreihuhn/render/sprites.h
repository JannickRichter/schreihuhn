#pragma once

#include <array>
#include <cstdint>
#include "schreihuhn/game/player.h"
#include "schreihuhn/hal/canvas155.h"
#include "schreihuhn/persistence/save_data.h"
#include "schreihuhn/render/palette.h"
#include "schreihuhn/world/world_types.h"

namespace schreihuhn::render {

using RemapTable = std::array<uint8_t, 16>;

class Sprites {
public:
    static void drawSprite(hal::Canvas155& canvas,
                           int x, int y,
                           int w, int h,
                           const uint8_t* pixels,
                           int scale = 1);

    static void drawSpriteRemap(hal::Canvas155& canvas,
                                int x, int y,
                                int w, int h,
                                const uint8_t* pixels,
                                const RemapTable& remap,
                                int scale = 1);

    static void drawChicken(hal::Canvas155& canvas,
                            int x, int y,
                            persistence::ChickenId chicken,
                            game::PlayerAnim anim,
                            uint8_t frame,
                            bool locked_silhouette,
                            int scale = 1);

    static void drawCoin(hal::Canvas155& canvas, int x, int y, uint8_t frame);
    static void drawCoinSparkle(hal::Canvas155& canvas, int x, int y, uint8_t frame);
    static void drawPerkPacket(hal::Canvas155& canvas, int x, int y, uint8_t pulse_frame);
    static void drawObstacle(hal::Canvas155& canvas, int x, int y,
                             world::ObstacleType type,
                             world::BiomeId biome = world::BiomeId::Greenland);
    static void drawDecor(hal::Canvas155& canvas, int ground_x, int ground_y,
                          world::DecorKind kind);
    static void drawBridgePlank(hal::Canvas155& canvas, int x, int y);

    static void drawCoinSmall(hal::Canvas155& canvas, int x, int y);
    static void drawLock(hal::Canvas155& canvas, int x, int y);
    static void drawSpeaker(hal::Canvas155& canvas, int x, int y, bool on);
    static void drawPauseIcon(hal::Canvas155& canvas, int x, int y, uint8_t color);
    static void drawArrowLeft(hal::Canvas155& canvas, int x, int y, uint8_t color);
    static void drawArrowRight(hal::Canvas155& canvas, int x, int y, uint8_t color);
};

} // namespace schreihuhn::render
