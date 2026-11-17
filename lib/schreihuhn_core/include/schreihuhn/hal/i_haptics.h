#pragma once

#include <cstdint>

namespace schreihuhn::hal {

class IHaptics {
public:
    virtual ~IHaptics() = default;
    virtual void vibrate(uint16_t duration_ms) = 0;
};

} // namespace schreihuhn::hal
