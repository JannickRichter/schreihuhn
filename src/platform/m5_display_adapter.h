#pragma once

#include <M5Unified.h>
#include <array>
#include <cstdint>
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/hal/canvas155.h"

namespace schreihuhn::platform {

class M5DisplayAdapter {
public:
    bool init();
    void present(const hal::Canvas155& canvas);

private:
    static constexpr int kScaledWidth = hal::Canvas155::kSize * config::K_DISPLAY_SCALE;

    void flushDisplay();

    std::array<uint16_t, 16> swapped_palette_{};
    std::array<uint16_t, static_cast<std::size_t>(kScaledWidth) * config::K_DISPLAY_SCALE> strip_buf_{};
    int32_t offset_x_ = 0;
    int32_t offset_y_ = 0;
    bool ready_ = false;
};

} // namespace schreihuhn::platform
