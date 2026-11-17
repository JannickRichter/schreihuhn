#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace schreihuhn::core {

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

struct RectI {
    int16_t x = 0;
    int16_t y = 0;
    int16_t w = 0;
    int16_t h = 0;

    constexpr bool contains(int16_t px, int16_t py) const {
        return px >= x && px < (x + w) && py >= y && py < (y + h);
    }
};

inline float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(v, hi));
}

inline float moveToward(float current, float target, float max_delta) {
    if (current < target) {
        return std::min(current + max_delta, target);
    }
    if (current > target) {
        return std::max(current - max_delta, target);
    }
    return target;
}

class XorShift32 {
public:
    explicit constexpr XorShift32(uint32_t seed = 1u)
        : state_(seed == 0u ? 0x9E3779B9u : seed) {}

    constexpr void reseed(uint32_t seed) {
        state_ = (seed == 0u) ? 0x9E3779B9u : seed;
    }

    constexpr uint32_t nextU32() {
        uint32_t x = state_;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state_ = x;
        return x;
    }

    constexpr int32_t rangeInt(int32_t min_inc, int32_t max_inc) {
        if (max_inc <= min_inc) {
            return min_inc;
        }
        const uint32_t span = static_cast<uint32_t>(max_inc - min_inc) + 1u;
        return min_inc + static_cast<int32_t>(nextU32() % span);
    }

    constexpr float nextFloat01() {
        return static_cast<float>(nextU32() >> 8) * (1.0f / 16777216.0f);
    }

private:
    uint32_t state_;
};

} // namespace schreihuhn::core
