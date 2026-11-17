#include "m5_haptics_adapter.h"

#include <M5Unified.h>

namespace schreihuhn::platform {

void M5HapticsAdapter::vibrate(uint16_t duration_ms) {
    if (duration_ms == 0) {
        return;
    }
    M5.Power.setVibration(180);
    active_ = true;
    stop_ms_ = millis() + duration_ms;
}

void M5HapticsAdapter::update(uint32_t now_ms) {
    if (active_ && static_cast<int32_t>(now_ms - stop_ms_) >= 0) {
        M5.Power.setVibration(0);
        active_ = false;
    }
}

} // namespace schreihuhn::platform
