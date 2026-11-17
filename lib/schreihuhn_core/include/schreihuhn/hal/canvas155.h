#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include "schreihuhn/config/tuning.h"

namespace schreihuhn::hal {

struct Canvas155 {
    static constexpr int kSize = config::K_LOGICAL_SIZE; // 155
    std::array<uint8_t, kSize * kSize> pixels{};

    void clear(uint8_t color) {
        pixels.fill(color);
    }

    void setPixel(int x, int y, uint8_t color) {
        if (x >= 0 && x < kSize && y >= 0 && y < kSize) {
            pixels[static_cast<std::size_t>(y * kSize + x)] = color;
        }
    }

    uint8_t getPixel(int x, int y) const {
        if (x >= 0 && x < kSize && y >= 0 && y < kSize) {
            return pixels[static_cast<std::size_t>(y * kSize + x)];
        }
        return 0;
    }

    void fillRect(int x, int y, int w, int h, uint8_t color) {
        if (w <= 0 || h <= 0) {
            return;
        }
        const int x0 = std::max(0, x);
        const int y0 = std::max(0, y);
        const int x1 = std::min(kSize, x + w);
        const int y1 = std::min(kSize, y + h);
        for (int py = y0; py < y1; ++py) {
            uint8_t* row = &pixels[static_cast<std::size_t>(py * kSize)];
            for (int px = x0; px < x1; ++px) {
                row[px] = color;
            }
        }
    }

    void drawRectOutline(int x, int y, int w, int h, uint8_t color) {
        if (w <= 0 || h <= 0) {
            return;
        }
        fillRect(x, y, w, 1, color);
        fillRect(x, y + h - 1, w, 1, color);
        fillRect(x, y, 1, h, color);
        fillRect(x + w - 1, y, 1, h, color);
    }

    void drawDitherOverlay(uint8_t color = 1) {
        for (int y = 0; y < kSize; ++y) {
            uint8_t* row = &pixels[static_cast<std::size_t>(y * kSize)];
            for (int x = (y & 1); x < kSize; x += 2) {
                row[x] = color;
            }
        }
    }

    void drawDitherRect(int x, int y, int w, int h, uint8_t color) {
        if (w <= 0 || h <= 0) {
            return;
        }
        const int x0 = std::max(0, x);
        const int y0 = std::max(0, y);
        const int x1 = std::min(kSize, x + w);
        const int y1 = std::min(kSize, y + h);
        for (int py = y0; py < y1; ++py) {
            uint8_t* row = &pixels[static_cast<std::size_t>(py * kSize)];
            for (int px = x0; px < x1; ++px) {
                if (((px + py) & 1) == 0) {
                    row[px] = color;
                }
            }
        }
    }
};

} // namespace schreihuhn::hal
