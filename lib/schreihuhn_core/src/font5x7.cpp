#include "schreihuhn/render/font5x7.h"

#include <array>

namespace schreihuhn::render {

namespace {

struct Glyph {
    uint8_t cols[5]; // 5 columns, bit 0 = top row, bit 6 = bottom row
};

// Compact 5x7 bitmap font for uppercase UI, digits, punctuation, and German umlauts (FR-GR-05, FR-UI-P8)
constexpr Glyph kBlank = {{0x00, 0x00, 0x00, 0x00, 0x00}};

Glyph lookupCodepoint(uint32_t cp) {
    switch (cp) {
        case ' ': return kBlank;
        case '!': return {{0x00, 0x00, 0x5F, 0x00, 0x00}};
        case '+': return {{0x08, 0x08, 0x3E, 0x08, 0x08}};
        case ',': return {{0x00, 0x50, 0x30, 0x00, 0x00}};
        case '-': return {{0x08, 0x08, 0x08, 0x08, 0x08}};
        case '.': return {{0x00, 0x60, 0x60, 0x00, 0x00}};
        case '/': return {{0x20, 0x10, 0x08, 0x04, 0x02}};
        case ':': return {{0x00, 0x36, 0x36, 0x00, 0x00}};
        case '=': return {{0x14, 0x14, 0x14, 0x14, 0x14}};
        case '?': return {{0x02, 0x01, 0x51, 0x09, 0x06}};
        case '0': return {{0x3E, 0x51, 0x49, 0x45, 0x3E}};
        case '1': return {{0x00, 0x42, 0x7F, 0x40, 0x00}};
        case '2': return {{0x42, 0x61, 0x51, 0x49, 0x46}};
        case '3': return {{0x21, 0x41, 0x45, 0x4B, 0x31}};
        case '4': return {{0x18, 0x14, 0x12, 0x7F, 0x10}};
        case '5': return {{0x27, 0x45, 0x45, 0x45, 0x39}};
        case '6': return {{0x3C, 0x4A, 0x49, 0x49, 0x30}};
        case '7': return {{0x01, 0x71, 0x09, 0x05, 0x03}};
        case '8': return {{0x36, 0x49, 0x49, 0x49, 0x36}};
        case '9': return {{0x06, 0x49, 0x49, 0x29, 0x1E}};
        case 'A': return {{0x7E, 0x11, 0x11, 0x11, 0x7E}};
        case 'B': return {{0x7F, 0x49, 0x49, 0x49, 0x36}};
        case 'C': return {{0x3E, 0x41, 0x41, 0x41, 0x22}};
        case 'D': return {{0x7F, 0x41, 0x41, 0x22, 0x1C}};
        case 'E': return {{0x7F, 0x49, 0x49, 0x49, 0x41}};
        case 'F': return {{0x7F, 0x09, 0x09, 0x09, 0x01}};
        case 'G': return {{0x3E, 0x41, 0x49, 0x49, 0x7A}};
        case 'H': return {{0x7F, 0x08, 0x08, 0x08, 0x7F}};
        case 'I': return {{0x00, 0x41, 0x7F, 0x41, 0x00}};
        case 'J': return {{0x20, 0x40, 0x41, 0x3F, 0x01}};
        case 'K': return {{0x7F, 0x08, 0x14, 0x22, 0x41}};
        case 'L': return {{0x7F, 0x40, 0x40, 0x40, 0x40}};
        case 'M': return {{0x7F, 0x02, 0x0C, 0x02, 0x7F}};
        case 'N': return {{0x7F, 0x04, 0x08, 0x10, 0x7F}};
        case 'O': return {{0x3E, 0x41, 0x41, 0x41, 0x3E}};
        case 'P': return {{0x7F, 0x09, 0x09, 0x09, 0x06}};
        case 'Q': return {{0x3E, 0x41, 0x51, 0x21, 0x5E}};
        case 'R': return {{0x7F, 0x09, 0x19, 0x29, 0x46}};
        case 'S': return {{0x46, 0x49, 0x49, 0x49, 0x31}};
        case 'T': return {{0x01, 0x01, 0x7F, 0x01, 0x01}};
        case 'U': return {{0x3F, 0x40, 0x40, 0x40, 0x3F}};
        case 'V': return {{0x1F, 0x20, 0x40, 0x20, 0x1F}};
        case 'W': return {{0x3F, 0x40, 0x38, 0x40, 0x3F}};
        case 'X': return {{0x63, 0x14, 0x08, 0x14, 0x63}};
        case 'Y': return {{0x07, 0x08, 0x70, 0x08, 0x07}};
        case 'Z': return {{0x61, 0x51, 0x49, 0x45, 0x43}};
        case 'm': return {{0x7C, 0x04, 0x18, 0x04, 0x78}}; // lowercase 'm' for meters (spec/06)
        case 0xC4: return {{0x7C, 0x15, 0x14, 0x15, 0x7C}}; // Ä (dots top + A)
        case 0xD6: return {{0x3C, 0x45, 0x44, 0x45, 0x3C}}; // Ö (dots top + O)
        case 0xDC: return {{0x3C, 0x41, 0x40, 0x41, 0x3C}}; // Ü (dots top + U)
        default:
            if (cp >= 'a' && cp <= 'z') {
                return lookupCodepoint(cp - 'a' + 'A');
            }
            return kBlank;
    }
}

uint32_t nextUtf8(const char*& p) {
    const unsigned char c0 = static_cast<unsigned char>(*p);
    if (c0 == 0) {
        return 0;
    }
    ++p;
    if ((c0 & 0x80u) == 0u) {
        return c0;
    }
    if ((c0 & 0xE0u) == 0xC0u) {
        const unsigned char c1 = static_cast<unsigned char>(*p);
        if (c1 == 0) {
            return '?';
        }
        ++p;
        const uint32_t cp = (static_cast<uint32_t>(c0 & 0x1Fu) << 6) |
                            static_cast<uint32_t>(c1 & 0x3Fu);
        if (cp == 0xE4) return 0xC4; // ä -> Ä
        if (cp == 0xF6) return 0xD6; // ö -> Ö
        if (cp == 0xFC) return 0xDC; // ü -> Ü
        return cp;
    }
    return '?';
}

void drawGlyph(hal::Canvas155& canvas, int x, int y, const Glyph& g, uint8_t color, int scale) {
    for (int col = 0; col < 5; ++col) {
        const uint8_t bits = g.cols[col];
        for (int row = 0; row < 7; ++row) {
            if ((bits & static_cast<uint8_t>(1u << row)) != 0u) {
                if (scale <= 1) {
                    canvas.setPixel(x + col, y + row, color);
                } else {
                    canvas.fillRect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

} // namespace

int Font5x7::measureWidth(const char* utf8, int scale) {
    if (utf8 == nullptr || *utf8 == '\0') {
        return 0;
    }
    int count = 0;
    const char* p = utf8;
    while (nextUtf8(p) != 0) {
        ++count;
    }
    if (count == 0) {
        return 0;
    }
    return (count * 6 - 1) * scale;
}

void Font5x7::drawText(hal::Canvas155& canvas,
                       int x, int y,
                       const char* utf8,
                       uint8_t color,
                       int scale) {
    if (utf8 == nullptr) {
        return;
    }
    int cx = x;
    const char* p = utf8;
    while (true) {
        const uint32_t cp = nextUtf8(p);
        if (cp == 0) {
            break;
        }
        const Glyph g = lookupCodepoint(cp);
        drawGlyph(canvas, cx, y, g, color, scale);
        cx += 6 * scale;
    }
}

void Font5x7::drawTextOutlined(hal::Canvas155& canvas,
                               int x, int y,
                               const char* utf8,
                               uint8_t color,
                               uint8_t outline_color,
                               int scale) {
    drawText(canvas, x - 1, y, utf8, outline_color, scale);
    drawText(canvas, x + 1, y, utf8, outline_color, scale);
    drawText(canvas, x, y - 1, utf8, outline_color, scale);
    drawText(canvas, x, y + 1, utf8, outline_color, scale);
    drawText(canvas, x, y, utf8, color, scale);
}

void Font5x7::drawTextCentered(hal::Canvas155& canvas,
                               int center_x, int y,
                               const char* utf8,
                               uint8_t color,
                               uint8_t outline_color,
                               int scale) {
    const int w = measureWidth(utf8, scale);
    const int x = center_x - (w / 2);
    drawTextOutlined(canvas, x, y, utf8, color, outline_color, scale);
}

} // namespace schreihuhn::render
