#pragma once

#include <cstdint>
#include "schreihuhn/hal/i_haptics.h"

namespace schreihuhn::platform {

class M5HapticsAdapter : public hal::IHaptics {
public:
    void vibrate(uint16_t duration_ms) override;
    void update(uint32_t now_ms);

private:
    bool active_ = false;
    uint32_t stop_ms_ = 0;
};

} // namespace schreihuhn::platform
