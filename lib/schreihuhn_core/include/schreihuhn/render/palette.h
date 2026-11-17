#pragma once

#include <array>
#include <cstdint>

namespace schreihuhn::render {

inline constexpr uint8_t K_TRANSPARENT = 0xFF;

// PICO-8 Palette indices (spec/08 8.2)
enum ColorIdx : uint8_t {
    C_BLACK       = 0,
    C_DARK_BLUE   = 1,
    C_DARK_PURPLE = 2,
    C_DARK_GREEN  = 3,
    C_BROWN       = 4,
    C_DARK_GREY   = 5,
    C_LIGHT_GREY  = 6,
    C_WHITE       = 7,
    C_RED         = 8,
    C_ORANGE      = 9,
    C_YELLOW      = 10,
    C_GREEN       = 11,
    C_LIGHT_BLUE  = 12,
    C_LAVENDER    = 13,
    C_PINK        = 14,
    C_PEACH       = 15
};

inline constexpr std::array<uint16_t, 16> K_PALETTE_RGB565 = {
    0x0000, //  0: #000000 Schwarz
    0x194A, //  1: #1D2B53 Dunkelblau
    0x792A, //  2: #7E2553 Dunkelviolett
    0x042A, //  3: #008751 Dunkelgruen
    0xAA86, //  4: #AB5236 Braun
    0x5AA9, //  5: #5F574F Dunkelgrau
    0xC618, //  6: #C2C3C7 Hellgrau
    0xFF9D, //  7: #FFF1E8 Weiss
    0xF809, //  8: #FF004D Rot
    0xFD00, //  9: #FFA300 Orange
    0xFF64, // 10: #FFEC27 Gelb
    0x0726, // 11: #00E436 Gruen
    0x2D7F, // 12: #29ADFF Hellblau
    0x83B3, // 13: #83769C Lavendel
    0xFBB5, // 14: #FF77A8 Rosa
    0xFE75  // 15: #FFCCAA Pfirsich
};

} // namespace schreihuhn::render
