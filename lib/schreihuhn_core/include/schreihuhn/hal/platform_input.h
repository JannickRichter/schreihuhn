#pragma once

#include <cstdint>

namespace schreihuhn::hal {

struct PlatformInput {
    bool btn_left_down  = false;
    bool btn_right_down = false;
    bool touch_down     = false;
    int16_t touch_x     = 0; // logical 0..154
    int16_t touch_y     = 0; // logical 0..154
    bool sim_talk       = false; // SIM_INPUT override
    bool sim_shout      = false; // SIM_INPUT override
};

} // namespace schreihuhn::hal
