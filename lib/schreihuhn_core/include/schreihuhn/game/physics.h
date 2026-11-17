#pragma once

#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/game/player.h"
#include "schreihuhn/world/world_types.h"

namespace schreihuhn::game {

struct PhysicsStepResult {
    bool landed_this_tick  = false;
    bool jumped_this_tick  = false;
    bool died_void         = false;
    bool hit_wall          = false;
};

class Physics {
public:
    static PhysicsStepResult step(PlayerState& player,
                                  world::WorldState& world,
                                  const audio::MicClassification& voice,
                                  float dt);

    static void updateAnimation(PlayerState& player, float dt);
};

} // namespace schreihuhn::game
