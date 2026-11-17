#pragma once

#include "schreihuhn/game/run_session.h"
#include "schreihuhn/hal/canvas155.h"
#include "schreihuhn/persistence/save_data.h"

namespace schreihuhn::render {

class WorldRenderer {
public:
    static void render(hal::Canvas155& canvas,
                       const game::RunSession& session,
                       persistence::ChickenId chicken,
                       float anim_clock_sec);

    // Renders static preview world (sky, parallax) for Menu/Countdown
    static void renderSkyAndParallax(hal::Canvas155& canvas, double camera_x);

    static void renderAtmosphere(hal::Canvas155& canvas,
                                 double camera_x,
                                 world::BiomeId prev_biome,
                                 world::BiomeId cur_biome,
                                 float transition_progress,
                                 float anim_clock_sec);
};

} // namespace schreihuhn::render
