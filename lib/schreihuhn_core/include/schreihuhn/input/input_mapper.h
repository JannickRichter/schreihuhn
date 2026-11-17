#pragma once

#include <cstdint>
#include "schreihuhn/core/math_types.h"
#include "schreihuhn/hal/platform_input.h"

namespace schreihuhn::input {

struct InputFrameEvents {
    bool left_short  = false;
    bool left_long   = false;
    bool right_short = false;
    bool right_long  = false;
    bool tap_valid   = false;
    int16_t tap_press_x   = 0;
    int16_t tap_press_y   = 0;
    int16_t tap_release_x = 0;
    int16_t tap_release_y = 0;

    bool left_visual_down  = false;
    bool right_visual_down = false;
    bool touch_visual_down = false;
    int16_t touch_curr_x   = 0;
    int16_t touch_curr_y   = 0;

    bool tapInRect(const core::RectI& r) const {
        return tap_valid &&
               r.contains(tap_press_x, tap_press_y) &&
               r.contains(tap_release_x, tap_release_y);
    }

    bool touchHeldInRect(const core::RectI& r) const {
        return touch_visual_down && r.contains(touch_curr_x, touch_curr_y);
    }

    bool anyKeyOrTap() const {
        return left_short || left_long || right_short || right_long || tap_valid;
    }
};

class InputMapper {
public:
    void onScreenChanged(uint16_t lockout_ms);
    InputFrameEvents update(const hal::PlatformInput& in, uint32_t dt_ms);

    bool isLockedOut() const { return lockout_remaining_ms_ > 0; }

private:
    uint32_t lockout_remaining_ms_ = 0;

    bool prev_left_down_  = false;
    bool prev_right_down_ = false;
    bool prev_touch_down_ = false;

    bool wait_release_left_  = false;
    bool wait_release_right_ = false;
    bool wait_release_touch_ = false;

    uint32_t left_held_ms_  = 0;
    uint32_t right_held_ms_ = 0;
    bool left_long_fired_   = false;
    bool right_long_fired_  = false;

    bool touch_active_      = false;
    uint32_t touch_held_ms_ = 0;
    int16_t touch_press_x_  = 0;
    int16_t touch_press_y_  = 0;
    int16_t touch_curr_x_   = 0;
    int16_t touch_curr_y_   = 0;
};

} // namespace schreihuhn::input
