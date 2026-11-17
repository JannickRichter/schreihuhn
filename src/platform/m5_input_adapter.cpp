#include "m5_input_adapter.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/hal/canvas155.h"

#include <algorithm>

namespace schreihuhn::platform {

hal::PlatformInput M5InputAdapter::poll(app::AppState app_state) const {
    hal::PlatformInput out{};

    const bool btn_a = M5.BtnA.isPressed();
    const bool btn_b = M5.BtnB.isPressed();
    out.btn_left_down  = config::K_BTN_LEFT_IS_BTNA ? btn_a : btn_b;
    out.btn_right_down = config::K_BTN_LEFT_IS_BTNA ? btn_b : btn_a;

    if (M5.Touch.getCount() > 0) {
        const auto t = M5.Touch.getDetail(0);
        out.touch_down = t.isPressed();
        const int32_t scaled = config::K_LOGICAL_SIZE * config::K_DISPLAY_SCALE;
        const int32_t offset_x = std::max<int32_t>(0, (M5.Display.width() - scaled) / 2);
        const int32_t offset_y = std::max<int32_t>(0, (M5.Display.height() - scaled) / 2);
        const int16_t lx = static_cast<int16_t>((t.x - offset_x) / config::K_DISPLAY_SCALE);
        const int16_t ly = static_cast<int16_t>((t.y - offset_y) / config::K_DISPLAY_SCALE);
        out.touch_x = std::clamp<int16_t>(lx, 0, hal::Canvas155::kSize - 1);
        out.touch_y = std::clamp<int16_t>(ly, 0, hal::Canvas155::kSize - 1);
    }

#ifdef SIM_INPUT
    if (app_state == app::AppState::Playing) {
        out.sim_talk  = out.btn_left_down;
        out.sim_shout = out.btn_right_down;
        out.btn_left_down  = false;
        out.btn_right_down = false;
    }
#else
    (void)app_state;
#endif

    return out;
}

} // namespace schreihuhn::platform
