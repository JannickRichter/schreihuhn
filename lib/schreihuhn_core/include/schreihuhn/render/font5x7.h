#pragma once

#include <cstdint>
#include "schreihuhn/hal/canvas155.h"

namespace schreihuhn::render {

class Font5x7 {
public:
    static int measureWidth(const char* utf8, int scale = 1);

    static void drawText(hal::Canvas155& canvas,
                         int x, int y,
                         const char* utf8,
                         uint8_t color,
                         int scale = 1);

    static void drawTextOutlined(hal::Canvas155& canvas,
                                 int x, int y,
                                 const char* utf8,
                                 uint8_t color,
                                 uint8_t outline_color,
                                 int scale = 1);

    static void drawTextCentered(hal::Canvas155& canvas,
                                 int center_x, int y,
                                 const char* utf8,
                                 uint8_t color,
                                 uint8_t outline_color,
                                 int scale = 1);
};

} // namespace schreihuhn::render
