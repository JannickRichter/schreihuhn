#include "schreihuhn/input/input_mapper.h"
#include "schreihuhn/config/tuning.h"

#include <cstdlib>

namespace schreihuhn::input {

void InputMapper::onScreenChanged(uint16_t lockout_ms) {
    lockout_remaining_ms_ = lockout_ms;

    if (prev_left_down_) {
        wait_release_left_ = true;
    }
    if (prev_right_down_) {
        wait_release_right_ = true;
    }
    if (prev_touch_down_) {
        wait_release_touch_ = true;
    }

    left_held_ms_   = 0;
    right_held_ms_  = 0;
    left_long_fired_  = false;
    right_long_fired_ = false;
    touch_active_   = false;
    touch_held_ms_  = 0;
}

InputFrameEvents InputMapper::update(const hal::PlatformInput& in, uint32_t dt_ms) {
    InputFrameEvents ev{};

    if (lockout_remaining_ms_ > 0) {
        if (dt_ms >= lockout_remaining_ms_) {
            lockout_remaining_ms_ = 0;
        } else {
            lockout_remaining_ms_ -= dt_ms;
        }
    }

    const bool locked = (lockout_remaining_ms_ > 0);

    // Clear wait-for-release flags once released
    if (!in.btn_left_down) {
        if (!prev_left_down_) {
            wait_release_left_ = false;
        }
    }
    if (!in.btn_right_down) {
        if (!prev_right_down_) {
            wait_release_right_ = false;
        }
    }
    if (!in.touch_down) {
        if (!prev_touch_down_) {
            wait_release_touch_ = false;
        }
    }

    // If locked out, suppress new presses until released
    if (locked) {
        if (in.btn_left_down && !prev_left_down_) {
            wait_release_left_ = true;
        }
        if (in.btn_right_down && !prev_right_down_) {
            wait_release_right_ = true;
        }
        if (in.touch_down && !prev_touch_down_) {
            wait_release_touch_ = true;
        }
        if (!in.btn_left_down) {
            wait_release_left_ = false;
        }
        if (!in.btn_right_down) {
            wait_release_right_ = false;
        }
        if (!in.touch_down) {
            wait_release_touch_ = false;
        }
        prev_left_down_  = in.btn_left_down;
        prev_right_down_ = in.btn_right_down;
        prev_touch_down_ = in.touch_down;
        return ev;
    }

    // Suppress simultaneous press of both hardware buttons until both are cleanly released
    if (in.btn_left_down && in.btn_right_down) {
        wait_release_left_  = true;
        wait_release_right_ = true;
        left_held_ms_       = 0;
        right_held_ms_      = 0;
        left_long_fired_    = false;
        right_long_fired_   = false;
    }

    // --- Left button (FR-UI-P5) ---
    if (in.btn_left_down) {
        if (!prev_left_down_) {
            if (!wait_release_left_) {
                left_held_ms_ = 0;
                left_long_fired_ = false;
            }
        } else if (!wait_release_left_) {
            left_held_ms_ += dt_ms;
            if (!left_long_fired_ && left_held_ms_ >= config::K_LONGPRESS_MS) {
                left_long_fired_ = true;
                ev.left_long = true;
            }
        }
        ev.left_visual_down = !wait_release_left_;
    } else {
        if (prev_left_down_ && !wait_release_left_ && !left_long_fired_) {
            ev.left_short = true;
        }
        wait_release_left_ = false;
        left_held_ms_ = 0;
        left_long_fired_ = false;
    }

    // --- Right button (FR-UI-P5) ---
    if (in.btn_right_down) {
        if (!prev_right_down_) {
            if (!wait_release_right_) {
                right_held_ms_ = 0;
                right_long_fired_ = false;
            }
        } else if (!wait_release_right_) {
            right_held_ms_ += dt_ms;
            if (!right_long_fired_ && right_held_ms_ >= config::K_LONGPRESS_MS) {
                right_long_fired_ = true;
                ev.right_long = true;
            }
        }
        ev.right_visual_down = !wait_release_right_;
    } else {
        if (prev_right_down_ && !wait_release_right_ && !right_long_fired_) {
            ev.right_short = true;
        }
        wait_release_right_ = false;
        right_held_ms_ = 0;
        right_long_fired_ = false;
    }

    // --- Touch tap (FR-UI-P4) ---
    if (in.touch_down) {
        if (!prev_touch_down_) {
            if (!wait_release_touch_) {
                touch_active_  = true;
                touch_held_ms_ = 0;
                touch_press_x_ = in.touch_x;
                touch_press_y_ = in.touch_y;
                touch_curr_x_  = in.touch_x;
                touch_curr_y_  = in.touch_y;
            }
        } else if (touch_active_ && !wait_release_touch_) {
            touch_held_ms_ += dt_ms;
            touch_curr_x_ = in.touch_x;
            touch_curr_y_ = in.touch_y;
        }
        if (touch_active_ && !wait_release_touch_) {
            ev.touch_visual_down = true;
            ev.touch_curr_x = touch_curr_x_;
            ev.touch_curr_y = touch_curr_y_;
        }
    } else {
        if (prev_touch_down_ && touch_active_ && !wait_release_touch_) {
            const int dx = std::abs(static_cast<int>(touch_curr_x_) - static_cast<int>(touch_press_x_));
            const int dy = std::abs(static_cast<int>(touch_curr_y_) - static_cast<int>(touch_press_y_));
            if (touch_held_ms_ <= config::K_TAP_MAX_MS &&
                dx < config::K_TAP_MAX_MOVE_PX &&
                dy < config::K_TAP_MAX_MOVE_PX) {
                ev.tap_valid     = true;
                ev.tap_press_x   = touch_press_x_;
                ev.tap_press_y   = touch_press_y_;
                ev.tap_release_x = touch_curr_x_;
                ev.tap_release_y = touch_curr_y_;
            }
        }
        touch_active_ = false;
        touch_held_ms_ = 0;
        wait_release_touch_ = false;
    }

    prev_left_down_  = in.btn_left_down;
    prev_right_down_ = in.btn_right_down;
    prev_touch_down_ = in.touch_down;
    return ev;
}

} // namespace schreihuhn::input
