#include "schreihuhn/render/world_renderer.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/render/palette.h"
#include "schreihuhn/render/sprites.h"

#include <algorithm>
#include <cmath>

namespace schreihuhn::render {

namespace {

inline constexpr uint8_t kBayer4[4][4] = {
    { 0,  8,  2, 10},
    {12,  4, 14,  6},
    { 3, 11,  1,  9},
    {15,  7, 13,  5}
};

inline bool bayerPass(int x, int y, int threshold16) {
    if (threshold16 <= 0) return false;
    if (threshold16 >= 16) return true;
    return kBayer4[y & 3][x & 3] < static_cast<uint8_t>(threshold16);
}

uint8_t skyPixelColor(world::BiomeId biome, int x, int y, double cam_x, float clock_sec) {
    switch (biome) {
        case world::BiomeId::Greenland: {
            if (y < 104) return C_LIGHT_BLUE;
            if (y < 135) {
                const int phase = y - 104;
                bool dark = false;
                if (phase < 10) dark = ((x + y) % 4 == 0) && ((y & 1) == 0);
                else if (phase < 21) dark = ((x + y) & 1) == 0;
                else dark = !(((x + y) % 4 == 0) && ((y & 1) == 0));
                return dark ? C_DARK_BLUE : C_LIGHT_BLUE;
            }
            if (y >= 145 && ((x + y) & 1) == 1 && (y >= 150 || ((x + y) % 4 == 0))) {
                return C_BLACK;
            }
            return C_DARK_BLUE;
        }

        case world::BiomeId::Taiga: {
            if (y < 60) return C_DARK_BLUE;
            if (y < 92) {
                const int thr = ((y - 60) * 16) / 32;
                return bayerPass(x, y, thr) ? C_LAVENDER : C_DARK_BLUE;
            }
            if (y < 120) return C_LAVENDER;
            if (y < 140) {
                const int thr = ((y - 120) * 16) / 20;
                return bayerPass(x, y, thr) ? C_DARK_BLUE : C_LAVENDER;
            }
            if (y >= 146 && bayerPass(x, y, ((y - 145) * 16) / 10)) {
                return C_BLACK;
            }
            return C_DARK_BLUE;
        }

        case world::BiomeId::Snow: {
            if (y < 54) return C_DARK_BLUE;
            if (y < 90) {
                const int thr = ((y - 54) * 16) / 36;
                return bayerPass(x, y, thr) ? C_LIGHT_GREY : C_DARK_BLUE;
            }
            if (y < 118) return C_LIGHT_GREY;
            if (y < 138) {
                const int thr = ((y - 118) * 16) / 20;
                return bayerPass(x, y, thr) ? C_DARK_BLUE : C_LIGHT_GREY;
            }
            if (y >= 146 && bayerPass(x, y, ((y - 145) * 16) / 10)) {
                return C_BLACK;
            }
            return C_DARK_BLUE;
        }

        case world::BiomeId::Lava: {
            if (y < 38) return C_BLACK;
            if (y < 70) {
                const int thr = ((y - 38) * 16) / 32;
                return bayerPass(x, y, thr) ? C_DARK_PURPLE : C_BLACK;
            }
            if (y < 104) {
                const int thr = ((y - 70) * 16) / 34;
                return bayerPass(x, y, thr) ? C_RED : C_DARK_PURPLE;
            }
            if (y < 142) {
                if (y >= 122) {
                    const int thr = ((y - 122) * 10) / 20;
                    return bayerPass(x, y, thr) ? C_ORANGE : C_RED;
                }
                return C_RED;
            }
            const int flow = x + static_cast<int>(clock_sec * 18.0f + cam_x * 0.4);
            const int ripple = ((flow / 5) + (y * 3)) & 7;
            if (y == 142) {
                return (ripple < 3) ? C_YELLOW : C_ORANGE;
            }
            if (ripple == 0) return C_YELLOW;
            if (ripple <= 3) return C_ORANGE;
            return C_RED;
        }

        case world::BiomeId::Jungle: {
            // Diagonal sunbeams filtering through upper jungle canopy
            const int ray_phase = (x * 2 - y + static_cast<int>(cam_x * 0.15)) & 63;
            const bool in_sunbeam = (ray_phase < 10) && (y < 115);

            if (y < 34) {
                return (in_sunbeam && bayerPass(x, y, 5)) ? C_GREEN : C_DARK_BLUE;
            }
            if (y < 68) {
                if (in_sunbeam && bayerPass(x, y, 6)) {
                    return C_YELLOW;
                }
                const int thr = ((y - 34) * 16) / 34;
                return bayerPass(x, y, thr) ? C_DARK_GREEN : C_DARK_BLUE;
            }
            if (y < 96) {
                if (in_sunbeam && bayerPass(x, y, 5)) {
                    return C_GREEN;
                }
                return C_DARK_GREEN;
            }
            // Humid jungle fog / mist band across y = 96..132
            if (y < 132) {
                const int fog_wave = (x + static_cast<int>(clock_sec * 6.0f + cam_x * 0.25)) / 10;
                const int thr = (y < 116) ? (((y - 96) * 9) / 20) : (((132 - y) * 9) / 16);
                if (bayerPass(x + fog_wave, y, thr)) {
                    return C_LIGHT_GREY;
                }
                return C_DARK_GREEN;
            }
            if (y >= 145 && bayerPass(x, y, ((y - 144) * 16) / 10)) {
                return C_BLACK;
            }
            return C_DARK_GREEN;
        }

        case world::BiomeId::Sand: {
            if (y < 52) return C_LIGHT_BLUE;
            if (y < 86) {
                const int thr = ((y - 52) * 16) / 34;
                return bayerPass(x, y, thr) ? C_PEACH : C_LIGHT_BLUE;
            }
            if (y < 122) return C_PEACH;
            if (y < 142) {
                const int thr = ((y - 122) * 16) / 20;
                return bayerPass(x, y, thr) ? C_ORANGE : C_PEACH;
            }
            return C_ORANGE;
        }

        case world::BiomeId::Candy: {
            // Pulsing, flashing bright white / purple / pink / blue / yellow Candy mode
            static constexpr uint8_t kCandyPalette[8] = {
                C_DARK_PURPLE, C_PINK, C_WHITE, C_LAVENDER,
                C_LIGHT_BLUE,  C_PINK, C_YELLOW, C_WHITE
            };
            const int flash = static_cast<int>(clock_sec * 8.0f);
            const int band = (x + y * 2 + flash * 9) / 14;
            const int idx0 = (band & 7);
            const int idx1 = ((band + 1) & 7);
            const int sub = (x + y * 2 + flash * 9) % 14;
            const int thr = (sub * 16) / 14;
            return bayerPass(x, y, thr) ? kCandyPalette[idx1] : kCandyPalette[idx0];
        }

        default:
            return C_LIGHT_BLUE;
    }
}

void putDitheredPixel(hal::Canvas155& canvas, int x, int y, uint8_t color,
                      int dither_thresh, bool invert_mask) {
    if (dither_thresh < 16) {
        const bool pass = bayerPass(x, y, dither_thresh);
        if (invert_mask ? pass : !pass) {
            return;
        }
    } else if (invert_mask) {
        return;
    }
    canvas.setPixel(x, y, color);
}

void drawCloudThemed(hal::Canvas155& canvas, int x, int y,
                     world::BiomeId biome, int dither_thresh, bool invert_mask) {
    uint8_t main_col = C_WHITE;
    uint8_t base_col = C_LIGHT_GREY;
    if (biome == world::BiomeId::Taiga) {
        main_col = C_LIGHT_GREY;
        base_col = C_LAVENDER;
    } else if (biome == world::BiomeId::Lava) {
        main_col = C_DARK_GREY;
        base_col = C_BLACK;
    } else if (biome == world::BiomeId::Jungle) {
        main_col = C_LIGHT_GREY;
        base_col = C_GREEN;
    } else if (biome == world::BiomeId::Candy) {
        main_col = C_WHITE;
        base_col = C_PINK;
    }

    auto fillDitherRect = [&](int rx, int ry, int rw, int rh, uint8_t col) {
        for (int py = ry; py < ry + rh; ++py) {
            for (int px = rx; px < rx + rw; ++px) {
                putDitheredPixel(canvas, px, py, col, dither_thresh, invert_mask);
            }
        }
    };

    fillDitherRect(x + 4, y, 14, 2, main_col);
    fillDitherRect(x + 2, y + 2, 18, 2, main_col);
    fillDitherRect(x, y + 4, 22, 2, main_col);
    fillDitherRect(x + 2, y + 6, 18, 1, base_col);
}

void drawDesertSun(hal::Canvas155& canvas, float clock_sec,
                   int dither_thresh, bool invert_mask) {
    const int cx = 122;
    const int cy = 24;
    const int pulse = static_cast<int>(clock_sec * 3.0f) & 1;
    for (int dy = -12; dy <= 12; ++dy) {
        for (int dx = -12; dx <= 12; ++dx) {
            const int r2 = dx * dx + dy * dy;
            if (r2 <= 25) {
                putDitheredPixel(canvas, cx + dx, cy + dy, C_WHITE, dither_thresh, invert_mask);
            } else if (r2 <= 64) {
                putDitheredPixel(canvas, cx + dx, cy + dy, C_YELLOW, dither_thresh, invert_mask);
            } else if (r2 <= (pulse ? 130 : 108) && (((dx + dy) & 1) == 0)) {
                putDitheredPixel(canvas, cx + dx, cy + dy, C_YELLOW, dither_thresh, invert_mask);
            }
        }
    }
}

void drawIceberg(hal::Canvas155& canvas, int x, int base_y, int w, int h, int idx,
                 int dither_thresh, bool invert_mask) {
    // Distinct angular faceted iceberg silhouette with tabular shelf & jagged crystal peak
    const int peak_off = (idx % 2 == 0) ? (w * 35) / 100 : (w * 65) / 100;
    const int shelf_y  = (h * 42) / 100;
    for (int dy = 0; dy < h; ++dy) {
        int left_x = 0;
        int right_x = 0;
        if (dy < shelf_y) {
            // Sharp jagged main crystal spire
            const int span = (dy * (w / 3)) / std::max(1, shelf_y);
            left_x = x + peak_off - span / 2 - 2;
            right_x = x + peak_off + span + 2;
        } else {
            // Wide faceted tabular ice shoulder tapering down smoothly at edges
            const int d2 = dy - shelf_y;
            const int rem = std::max(1, h - shelf_y);
            const int half = (w * 35) / 100 + (d2 * (w * 15)) / (100 * rem);
            left_x = x + w / 2 - half;
            right_x = x + w / 2 + half;
            // Jagged crystal notch on top shelf edge
            if (dy < shelf_y + 4) {
                left_x += (dy - shelf_y);
            }
        }
        const int py = base_y - h + dy;
        const int split = x + peak_off + (dy / 4);
        for (int px = left_x; px < right_x; ++px) {
            uint8_t col = C_LIGHT_BLUE;
            if (px <= left_x + 1 || dy <= 2) {
                col = C_WHITE;
            } else if (px < split) {
                col = (dy < shelf_y + 3) ? C_WHITE : C_LIGHT_BLUE;
            } else {
                col = (px > right_x - 4) ? C_DARK_BLUE : C_LIGHT_GREY;
            }
            putDitheredPixel(canvas, px, py, col, dither_thresh, invert_mask);
        }
    }
}

void drawSandDune(hal::Canvas155& canvas, int x, int base_y, int w, int h,
                  int dither_thresh, bool invert_mask) {
    // Asymmetric rounded sand dune with sunlit crest and shaded leeward side
    const int crest = (w * 62) / 100;
    for (int dy = 0; dy < h; ++dy) {
        const float t = static_cast<float>(dy) / static_cast<float>(std::max(1, h));
        const float root = std::sqrt(t);
        const int left_in  = crest - static_cast<int>(root * static_cast<float>(crest));
        const int right_in = crest + static_cast<int>(root * static_cast<float>(w - crest));
        const int py = base_y - h + dy;
        const int ridge = crest + (dy / 4);
        for (int px = x + left_in; px <= x + right_in; ++px) {
            uint8_t col = C_PEACH;
            if (px < x + ridge - 1) {
                col = (dy < h / 3) ? C_YELLOW : C_PEACH;
            } else {
                col = C_ORANGE;
            }
            putDitheredPixel(canvas, px, py, col, dither_thresh, invert_mask);
        }
    }
}

void drawJungleSilhouette(hal::Canvas155& canvas, int x, int base_y, int idx,
                          int dither_thresh, bool invert_mask) {
    // Tall rainforest canopy tree + stepped distant Jungle Temple pyramid
    if (idx == 1) {
        // Stepped stone Maya/Aztec temple in distance
        for (int tier = 0; tier < 4; ++tier) {
            const int tw = 14 + tier * 10;
            const int th = 7;
            const int tx = x + 22 - tw / 2;
            const int ty = base_y - 28 + tier * th;
            for (int py = ty; py < ty + th; ++py) {
                for (int px = tx; px < tx + tw; ++px) {
                    uint8_t c = (py == ty || px == tx) ? C_LIGHT_GREY : C_DARK_GREY;
                    if (((px + py * 3) % 11) == 0) c = C_DARK_GREEN; // Overgrown vines
                    putDitheredPixel(canvas, px, py, c, dither_thresh, invert_mask);
                }
            }
        }
        return;
    }

    // Giant rainforest canopy tree silhouette
    const int trunk_x = x + 20;
    const int top_y = base_y - 54 - (idx * 6);
    for (int py = top_y + 12; py < base_y; ++py) {
        for (int px = trunk_x; px < trunk_x + 6; ++px) {
            putDitheredPixel(canvas, px, py, (px == trunk_x) ? C_BROWN : C_BLACK,
                             dither_thresh, invert_mask);
        }
    }
    // Layered domed canopy tiers
    const int crowns[2][3] = { {trunk_x - 14, top_y, 34}, {trunk_x - 8, top_y - 10, 22} };
    for (int c = 0; c < 2; ++c) {
        const int cx = crowns[c][0];
        const int cy = crowns[c][1];
        const int cw = crowns[c][2];
        for (int dy = 0; dy < 14; ++dy) {
            const int inset = std::abs(dy - 6) * 2;
            for (int px = cx + inset; px < cx + cw - inset; ++px) {
                const uint8_t col = (dy < 4) ? C_GREEN : ((dy < 10) ? C_DARK_GREEN : C_BLACK);
                putDitheredPixel(canvas, px, cy + dy, col, dither_thresh, invert_mask);
            }
        }
    }
}

void drawCandyParallaxItem(hal::Canvas155& canvas, int x, int base_y, int idx, float clock_sec,
                           int dither_thresh, bool invert_mask) {
    // Striped Candy Hill + Giant Swirl Lollipop silhouette
    const int flash = static_cast<int>(clock_sec * 6.0f + static_cast<float>(idx));
    const int h = 28 + (idx % 2) * 10;
    const int w = 58;
    for (int dy = 0; dy < h; ++dy) {
        const float ratio = static_cast<float>(h - dy) / static_cast<float>(h);
        const int inset = static_cast<int>(ratio * ratio * static_cast<float>(w / 2));
        for (int px = x + inset; px < x + w - inset; ++px) {
            const uint8_t col = (((px + dy + flash) / 5) & 1) ? C_PINK : C_LAVENDER;
            putDitheredPixel(canvas, px, base_y - h + dy, col, dither_thresh, invert_mask);
        }
    }

    // Giant lollipop rising behind the hill
    const int lx = x + 26;
    const int ly = base_y - h - 18;
    for (int py = ly + 10; py < base_y - h + 6; ++py) {
        putDitheredPixel(canvas, lx + 4, py, C_WHITE, dither_thresh, invert_mask);
        putDitheredPixel(canvas, lx + 5, py, C_LIGHT_GREY, dither_thresh, invert_mask);
    }
    for (int dy = -6; dy <= 6; ++dy) {
        for (int dx = -6; dx <= 6; ++dx) {
            if (dx * dx + dy * dy <= 36) {
                const bool ring = ((dx * dx + dy * dy + flash) / 6) & 1;
                const uint8_t c = ring ? C_WHITE : (((flash & 1) == 0) ? C_PINK : C_YELLOW);
                putDitheredPixel(canvas, lx + 5 + dx, ly + 4 + dy, c, dither_thresh, invert_mask);
            }
        }
    }
}

void drawMountainThemed(hal::Canvas155& canvas, int x, int base_y, int w, int h,
                        world::BiomeId biome, float clock_sec, int idx,
                        int dither_thresh, bool invert_mask) {
    if (biome == world::BiomeId::Snow) {
        drawIceberg(canvas, x, base_y, w, h, idx, dither_thresh, invert_mask);
        return;
    }
    if (biome == world::BiomeId::Sand) {
        drawSandDune(canvas, x, base_y, w, h, dither_thresh, invert_mask);
        return;
    }
    if (biome == world::BiomeId::Jungle) {
        drawJungleSilhouette(canvas, x, base_y, idx, dither_thresh, invert_mask);
        return;
    }
    if (biome == world::BiomeId::Candy) {
        drawCandyParallaxItem(canvas, x, base_y, idx, clock_sec, dither_thresh, invert_mask);
        return;
    }

    for (int dy = 0; dy < h; ++dy) {
        int inset = 0;
        if (biome == world::BiomeId::Greenland) {
            // Soft rounded rolling hills (quadratic dome curve)
            const float ratio = static_cast<float>(h - 1 - dy) / static_cast<float>(std::max(1, h));
            inset = static_cast<int>(ratio * ratio * static_cast<float>(w / 2));
        } else if (biome == world::BiomeId::Taiga) {
            // Asymmetric jagged crags
            const int jag = ((dy * 3 + idx * 5) % 5) - 2;
            inset = std::max(0, (h - 1 - dy) * (w / 2) / std::max(1, h) + jag);
        } else {
            inset = (h - 1 - dy) * (w / 2) / std::max(1, h);
        }
        const int rx = x + inset;
        const int rw = w - 2 * inset;
        if (rw <= 0) {
            continue;
        }
        const int py = base_y - h + dy;

        if (biome == world::BiomeId::Lava && dy < 4) {
            const int crater_w = std::max(4, rw - 2);
            const int cx = rx + (rw - crater_w) / 2;
            for (int px = cx; px < cx + crater_w; ++px) {
                uint8_t c = C_RED;
                if (dy <= 1) c = (((px + static_cast<int>(clock_sec * 8.0f)) & 3) == 0) ? C_YELLOW : C_ORANGE;
                else if (dy == 2) c = C_ORANGE;
                putDitheredPixel(canvas, px, py, c, dither_thresh, invert_mask);
            }
            continue;
        }

        for (int px = rx; px < rx + rw; ++px) {
            uint8_t col = C_GREEN;
            if (biome == world::BiomeId::Greenland) {
                col = (dy < h / 2) ? C_GREEN : C_DARK_GREEN;
            } else if (biome == world::BiomeId::Taiga) {
                const bool rocky_band = (dy < (h * 58) / 100);
                if (rocky_band) {
                    const bool rim = (px < rx + 2 || px > rx + rw - 3);
                    col = rim ? C_LIGHT_GREY : C_DARK_GREY;
                } else {
                    col = C_DARK_GREEN;
                }
            } else if (biome == world::BiomeId::Lava) {
                const int streak_x = rx + rw / 2 + ((dy * 2) / 5) - 1;
                if (dy >= 4 && dy < (h * 75) / 100 && std::abs(px - streak_x) <= 1) {
                    col = ((dy + static_cast<int>(clock_sec * 6.0f)) % 3 == 0) ? C_ORANGE : C_RED;
                } else {
                    col = (px < rx + 2) ? C_DARK_GREY : C_BLACK;
                }
            }
            putDitheredPixel(canvas, px, py, col, dither_thresh, invert_mask);
        }
    }

    if (biome == world::BiomeId::Lava) {
        const int puff_shift = static_cast<int>(clock_sec * 9.0f + static_cast<float>(idx) * 7.0f) % 14;
        const int sx = x + w / 2 - 4 - puff_shift / 3;
        const int sy = base_y - h - 4 - puff_shift;
        if (sy >= 8) {
            for (int px = sx; px < sx + 7; ++px) {
                putDitheredPixel(canvas, px, sy, C_DARK_GREY, dither_thresh, invert_mask);
                if (px > sx && px < sx + 5) {
                    putDitheredPixel(canvas, px, sy - 1, C_LIGHT_GREY, dither_thresh, invert_mask);
                }
            }
        }
    }
}

void drawParallaxLayerForBiome(hal::Canvas155& canvas, double camera_x,
                               world::BiomeId biome, float clock_sec,
                               int dither_thresh, bool invert_mask) {
    if (biome == world::BiomeId::Sand) {
        drawDesertSun(canvas, clock_sec, dither_thresh, invert_mask);
    }

    const int cloud_period = 160;
    const int cloud_off = static_cast<int>(std::fmod(camera_x * 0.2, static_cast<double>(cloud_period)));
    for (int rep = -1; rep <= 1; ++rep) {
        const int base = rep * cloud_period - cloud_off;
        if (biome != world::BiomeId::Sand) {
            drawCloudThemed(canvas, base + 14, 18, biome, dither_thresh, invert_mask);
        }
        if (biome != world::BiomeId::Sand && biome != world::BiomeId::Jungle) {
            drawCloudThemed(canvas, base + 76, 30, biome, dither_thresh, invert_mask);
        }
        drawCloudThemed(canvas, base + 124, 14, biome, dither_thresh, invert_mask);
    }

    const int hill_period = 160;
    const int hill_off = static_cast<int>(std::fmod(camera_x * 0.4, static_cast<double>(hill_period)));
    for (int rep = -1; rep <= 1; ++rep) {
        const int base = rep * hill_period - hill_off;
        if (biome == world::BiomeId::Snow) {
            drawMountainThemed(canvas, base + 2, 122, 62, 46, biome, clock_sec, 0, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 54, 124, 70, 52, biome, clock_sec, 1, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 110, 122, 54, 38, biome, clock_sec, 2, dither_thresh, invert_mask);
        } else if (biome == world::BiomeId::Lava) {
            drawMountainThemed(canvas, base + 4, 122, 56, 36, biome, clock_sec, 0, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 54, 124, 68, 44, biome, clock_sec, 1, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 112, 122, 48, 30, biome, clock_sec, 2, dither_thresh, invert_mask);
        } else if (biome == world::BiomeId::Sand) {
            drawMountainThemed(canvas, base + 0, 122, 74, 24, biome, clock_sec, 0, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 56, 124, 82, 30, biome, clock_sec, 1, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 118, 122, 66, 20, biome, clock_sec, 2, dither_thresh, invert_mask);
        } else if (biome == world::BiomeId::Jungle) {
            drawMountainThemed(canvas, base + 2, 124, 52, 54, biome, clock_sec, 0, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 56, 124, 54, 40, biome, clock_sec, 1, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 108, 124, 52, 58, biome, clock_sec, 2, dither_thresh, invert_mask);
        } else {
            drawMountainThemed(canvas, base + 5, 118, 56, 26, biome, clock_sec, 0, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 52, 120, 68, 34, biome, clock_sec, 1, dither_thresh, invert_mask);
            drawMountainThemed(canvas, base + 108, 118, 54, 22, biome, clock_sec, 2, dither_thresh, invert_mask);
        }
    }
}

void drawIsland(hal::Canvas155& canvas,
                const world::Island& isl,
                world::BiomeId render_biome,
                double camera_x,
                bool has_left_neighbor,
                bool has_right_neighbor,
                float clock_sec) {
    const int sx = static_cast<int>(std::lround(isl.x - camera_x));
    const int sw = static_cast<int>(std::lround(isl.w));
    const int sy = static_cast<int>(std::lround(isl.y));

    if (sx + sw <= 0 || sx >= hal::Canvas155::kSize) {
        return;
    }

    const int depth = 24;
    uint8_t top_col    = C_GREEN;
    uint8_t lip_col    = C_DARK_GREEN;
    uint8_t body_col   = C_BROWN;
    uint8_t speck_col  = C_DARK_GREY;
    uint8_t edge_col   = C_BLACK;

    switch (render_biome) {
        case world::BiomeId::Taiga:
            top_col   = C_DARK_GREEN;
            lip_col   = C_BROWN;
            body_col  = C_DARK_GREY;
            speck_col = C_LIGHT_GREY;
            break;
        case world::BiomeId::Snow:
            top_col   = C_WHITE;
            lip_col   = C_LIGHT_BLUE;
            body_col  = C_DARK_GREY;
            speck_col = C_LIGHT_BLUE;
            edge_col  = C_DARK_BLUE;
            break;
        case world::BiomeId::Lava:
            top_col   = C_DARK_GREY;
            lip_col   = C_RED;
            body_col  = C_BLACK;
            speck_col = C_DARK_GREY;
            edge_col  = C_BLACK;
            break;
        case world::BiomeId::Jungle:
            top_col   = C_DARK_GREEN;
            lip_col   = C_GREEN;
            body_col  = C_BROWN;
            speck_col = C_DARK_GREEN;
            break;
        case world::BiomeId::Sand:
            top_col   = C_YELLOW;
            lip_col   = C_PEACH;
            body_col  = C_ORANGE;
            speck_col = C_BROWN;
            edge_col  = C_BROWN;
            break;
        case world::BiomeId::Candy: {
            const bool blink = (static_cast<int>(clock_sec * 8.0f) & 1) == 0;
            top_col   = blink ? C_WHITE : C_PINK;
            lip_col   = blink ? C_PINK : C_LAVENDER;
            body_col  = C_LAVENDER;
            speck_col = C_YELLOW;
            edge_col  = C_DARK_PURPLE;
            break;
        }
        default:
            break;
    }

    // Top surface cap
    canvas.fillRect(sx, sy, sw, 1, edge_col);
    canvas.fillRect(sx, sy + 1, sw, 3, top_col);
    canvas.fillRect(sx, sy + 4, sw, 2, lip_col);

    for (int lx = 2; lx < sw - 2; lx += 6) {
        if (render_biome == world::BiomeId::Lava) {
            const uint8_t glow = (((lx / 6 + static_cast<int>(clock_sec * 4.0f)) & 1) == 0) ? C_ORANGE : C_RED;
            canvas.setPixel(sx + lx, sy + 2, glow);
            canvas.setPixel(sx + lx + 1, sy + 4, C_ORANGE);
        } else if (render_biome == world::BiomeId::Candy) {
            const uint8_t sprinkle = ((lx / 6) % 3 == 0) ? C_YELLOW : (((lx / 6) % 3 == 1) ? C_LIGHT_BLUE : C_WHITE);
            canvas.setPixel(sx + lx, sy + 2, sprinkle);
            canvas.setPixel(sx + lx + 2, sy + 3, C_DARK_PURPLE);
        } else if (render_biome == world::BiomeId::Jungle) {
            // Hanging lush jungle vines along island top lip
            canvas.setPixel(sx + lx, sy + 5, C_GREEN);
            canvas.setPixel(sx + lx, sy + 6, C_DARK_GREEN);
            if ((lx & 1) == 0) {
                canvas.setPixel(sx + lx, sy + 7, C_GREEN);
            }
        } else {
            canvas.setPixel(sx + lx, sy + 4, top_col);
            canvas.setPixel(sx + lx + 1, sy + 5, lip_col);
        }
    }

    // Cliff body tapering down (Snow has blockier glacial step silhouette)
    for (int dy = 6; dy < depth; ++dy) {
        int left_in = 0;
        int right_in = 0;
        if (dy >= 10) {
            const int taper = (render_biome == world::BiomeId::Snow)
                                  ? (((dy - 8) / 4) * 3)
                                  : ((dy - 9) * 3 / 4);
            if (!has_left_neighbor) {
                left_in = std::min(taper, sw / 3);
            }
            if (!has_right_neighbor) {
                right_in = std::min(taper, sw / 3);
            }
        }
        const int row_x = sx + left_in;
        const int row_w = sw - left_in - right_in;
        if (row_w <= 2) {
            break;
        }
        canvas.fillRect(row_x, sy + dy, row_w, 1, body_col);

        for (int px = row_x + 3; px < row_x + row_w - 3; px += 9) {
            if (render_biome == world::BiomeId::Lava) {
                if (((px + dy * 2) % 11) == 0) {
                    canvas.setPixel(px, sy + dy, C_RED);
                } else if (((px + dy) % 13) == 0) {
                    canvas.setPixel(px, sy + dy, C_ORANGE);
                }
            } else if (((px + dy) % 5) == 0) {
                canvas.setPixel(px, sy + dy, speck_col);
            }
        }

        if (!has_left_neighbor || left_in > 0) {
            canvas.setPixel(row_x, sy + dy, edge_col);
        }
        if (!has_right_neighbor || right_in > 0) {
            canvas.setPixel(row_x + row_w - 1, sy + dy, edge_col);
        }
        if (dy == depth - 1) {
            canvas.fillRect(row_x, sy + dy, row_w, 1, edge_col);
        }
    }

    if (!has_left_neighbor) {
        canvas.fillRect(sx, sy, 1, 10, edge_col);
    }
    if (!has_right_neighbor) {
        canvas.fillRect(sx + sw - 1, sy, 1, 10, edge_col);
    }
}

void drawAmbientParticles(hal::Canvas155& canvas, const game::RunSession& session,
                          float clock_sec) {
    const uint8_t count = session.ambientCount();
    if (count == 0) {
        return;
    }
    const auto& list = session.ambientParticles();
    const float tr = session.biomeTransitionProgress();
    const world::BiomeId cur = session.currentBiome();

    for (uint8_t i = 0; i < count; ++i) {
        const auto& pt = list[i];
        if (tr < 1.0f) {
            const float gate = (pt.biome == cur) ? tr : (1.0f - tr);
            const float cutoff = static_cast<float>(i + 1) / static_cast<float>(count);
            if (gate < cutoff) {
                continue;
            }
        }
        const int x = static_cast<int>(std::lround(pt.x));
        const int y = static_cast<int>(std::lround(pt.y));

        if (pt.kind == 0) {
            const uint8_t col = ((i & 3) == 0) ? C_WHITE : C_LIGHT_GREY;
            canvas.setPixel(x, y, col);
            if ((i & 3) == 0) {
                canvas.setPixel(x + 1, y, C_WHITE);
            }
        } else if (pt.kind == 1) {
            canvas.setPixel(x, y, C_YELLOW);
            canvas.setPixel(x + 1, y, C_YELLOW);
            canvas.setPixel(x + 1, y - 1, C_ORANGE);
            canvas.setPixel(x + 2, y - 2, C_ORANGE);
            canvas.setPixel(x + 3, y - 3, C_RED);
        } else if (pt.kind == 2) {
            const bool pulse = (static_cast<int>(clock_sec * 8.0f + pt.phase) & 1) == 0;
            canvas.setPixel(x, y, pulse ? C_ORANGE : C_RED);
        } else if (pt.kind == 3) {
            // Very light subtle Jungle rain streaks
            const uint8_t col = ((i & 1) == 0) ? C_LIGHT_BLUE : C_WHITE;
            canvas.setPixel(x, y, col);
            canvas.setPixel(x, y - 1, col);
        } else {
            // Candy mode flashing star confetti
            static constexpr uint8_t kStarCols[4] = {C_WHITE, C_PINK, C_YELLOW, C_LIGHT_BLUE};
            const uint8_t col = kStarCols[(static_cast<int>(clock_sec * 10.0f) + i) & 3];
            canvas.setPixel(x, y, col);
            if ((i & 1) == 0) {
                canvas.setPixel(x + 1, y, col);
                canvas.setPixel(x, y + 1, C_WHITE);
            }
        }
    }
}

} // namespace

void WorldRenderer::renderAtmosphere(hal::Canvas155& canvas,
                                     double camera_x,
                                     world::BiomeId prev_biome,
                                     world::BiomeId cur_biome,
                                     float transition_progress,
                                     float anim_clock_sec) {
    const int thresh = static_cast<int>(std::lround(
        std::clamp(transition_progress, 0.0f, 1.0f) * 16.0f));
    const bool same = (prev_biome == cur_biome) || (thresh >= 16);

    for (int y = 0; y < hal::Canvas155::kSize; ++y) {
        uint8_t* row = &canvas.pixels[static_cast<std::size_t>(y * hal::Canvas155::kSize)];
        for (int x = 0; x < hal::Canvas155::kSize; ++x) {
            if (same) {
                row[x] = skyPixelColor(cur_biome, x, y, camera_x, anim_clock_sec);
            } else {
                const bool use_new = bayerPass(x, y, thresh);
                row[x] = skyPixelColor(use_new ? cur_biome : prev_biome,
                                       x, y, camera_x, anim_clock_sec);
            }
        }
    }

    if (same) {
        drawParallaxLayerForBiome(canvas, camera_x, cur_biome, anim_clock_sec, 16, false);
    } else {
        drawParallaxLayerForBiome(canvas, camera_x, prev_biome, anim_clock_sec, thresh, true);
        drawParallaxLayerForBiome(canvas, camera_x, cur_biome, anim_clock_sec, thresh, false);
    }
}

void WorldRenderer::renderSkyAndParallax(hal::Canvas155& canvas, double camera_x) {
    renderAtmosphere(canvas, camera_x,
                     world::BiomeId::Greenland, world::BiomeId::Greenland,
                     1.0f, 0.0f);
}

void WorldRenderer::render(hal::Canvas155& canvas,
                           const game::RunSession& session,
                           persistence::ChickenId chicken,
                           float anim_clock_sec) {
    const double cam_x = session.cameraX();
    const auto& world = session.world();
    const auto& p = session.player();
    const bool candy_active = (p.perk_timer > 0.0f);

    // 1 & 2: Dynamic Biome Atmosphere + Parallax
    renderAtmosphere(canvas, cam_x,
                     session.prevBiome(),
                     session.currentBiome(),
                     session.biomeTransitionProgress(),
                     anim_clock_sec);

    // 2b. Background weather & candy particles
    drawAmbientParticles(canvas, session, anim_clock_sec);

    // 3a. Islands
    for (std::size_t i = 0; i < world.islands.size(); ++i) {
        const auto& isl = world.islands[i];
        bool has_left = false;
        bool has_right = false;
        if (i > 0) {
            const auto& prev = world.islands[i - 1];
            if (std::fabs(prev.y - isl.y) < 0.5f &&
                std::fabs((prev.x + prev.w) - isl.x) < 1.0) {
                has_left = true;
            }
        }
        if (i + 1 < world.islands.size()) {
            const auto& next = world.islands[i + 1];
            if (std::fabs(next.y - isl.y) < 0.5f &&
                std::fabs((isl.x + isl.w) - next.x) < 1.0) {
                has_right = true;
            }
        }
        const world::BiomeId render_biome = candy_active ? world::BiomeId::Candy : isl.biome;
        drawIsland(canvas, isl, render_biome, cam_x, has_left, has_right, anim_clock_sec);
    }

    // 3a-decor. Background non-colliding props (trees, temples, cacti, lollipops)
    for (std::size_t i = 0; i < world.decors.size(); ++i) {
        const auto& dec = world.decors[i];
        const int sx = static_cast<int>(std::lround(dec.x - cam_x));
        const int sy = static_cast<int>(std::lround(dec.y));
        if (sx + 24 <= 0 || sx >= hal::Canvas155::kSize || sy >= hal::Canvas155::kSize) {
            continue;
        }
        const world::DecorKind dk = candy_active
                                        ? ((i & 1u) == 0u ? world::DecorKind::CandyLollipop
                                                          : world::DecorKind::CandyCane)
                                        : dec.kind;
        Sprites::drawDecor(canvas, sx, sy, dk);
    }

    // 3b. Bridge segments (FR-GR-13)
    const double p_left = p.world_x;
    const double p_right = p.world_x + static_cast<double>(config::K_HITBOX_W);
    const float p_foot_y = p.y + config::K_HITBOX_H;
    const int shake_tick = static_cast<int>(anim_clock_sec * 30.0f);
    for (std::size_t i = 0; i < world.bridges.size(); ++i) {
        const auto& seg = world.bridges[i];
        const int sx = static_cast<int>(std::lround(seg.x - cam_x));
        int sy = static_cast<int>(std::lround(seg.y));
        if (sx + 8 <= 0 || sx >= hal::Canvas155::kSize || sy >= hal::Canvas155::kSize) {
            continue;
        }
        if (seg.intact && seg.stability < config::K_BRIDGE_STABILITY - 0.02f) {
            const bool player_on_seg = p.on_ground &&
                                       std::fabs(seg.y - p_foot_y) < 1.0f &&
                                       p_right > seg.x &&
                                       p_left < seg.x + static_cast<double>(config::K_BRIDGE_SEG_W);
            if (player_on_seg && seg.stability < config::K_BRIDGE_SHAKE) {
                sy += (((shake_tick + static_cast<int>(i)) & 1) != 0) ? 1 : 0;
            } else {
                sy += 1;
            }
        }
        Sprites::drawBridgePlank(canvas, sx, sy);
    }

    // 3c. Obstacles (FR-GR-14 & FR-OB-*)
    for (std::size_t i = 0; i < world.obstacles.size(); ++i) {
        const auto& obs = world.obstacles[i];
        const int sx = static_cast<int>(std::lround(obs.x - cam_x));
        const int sy = static_cast<int>(std::lround(obs.y));
        if (sx + static_cast<int>(obs.w) <= 0 || sx >= hal::Canvas155::kSize ||
            sy >= hal::Canvas155::kSize) {
            continue;
        }
        const world::BiomeId render_biome = candy_active ? world::BiomeId::Candy : obs.biome;
        Sprites::drawObstacle(canvas, sx, sy, obs.type, render_biome);
    }

    // 4. Coins (FR-MP-25)
    const uint8_t coin_frame = static_cast<uint8_t>(static_cast<int>(anim_clock_sec / 0.12f) % 4);
    for (std::size_t i = 0; i < world.coins.size(); ++i) {
        const auto& c = world.coins[i];
        if (c.collected) {
            continue;
        }
        const int sx = static_cast<int>(std::lround(c.x - cam_x));
        const int bob = static_cast<int>(std::lround(
            std::sin(anim_clock_sec * 5.0f + static_cast<float>(c.x) * 0.1f)));
        const int sy = static_cast<int>(std::lround(c.y)) + bob;
        if (sx + 8 <= 0 || sx >= hal::Canvas155::kSize) {
            continue;
        }
        Sprites::drawCoin(canvas, sx, sy, coin_frame);
    }

    // 4b. White-packet Perk pickups (FR-PK-01)
    const uint8_t perk_pulse = static_cast<uint8_t>(static_cast<int>(anim_clock_sec / 0.10f) % 4);
    for (std::size_t i = 0; i < world.perks.size(); ++i) {
        const auto& pk = world.perks[i];
        if (pk.collected) {
            continue;
        }
        const int sx = static_cast<int>(std::lround(pk.x - cam_x));
        const int bob = static_cast<int>(std::lround(
            std::sin(anim_clock_sec * 6.0f + static_cast<float>(pk.x) * 0.12f)));
        const int sy = static_cast<int>(std::lround(pk.y)) + bob;
        if (sx + 8 <= 0 || sx >= hal::Canvas155::kSize) {
            continue;
        }
        Sprites::drawPerkPacket(canvas, sx, sy, perk_pulse);
    }

    // Subtle visual cue for top ceiling barrier during perk flight
    if (candy_active) {
        const int ceil_y = static_cast<int>(config::K_PERK_CEILING_Y) - 1;
        const int shift = static_cast<int>(anim_clock_sec * 20.0f) & 7;
        for (int x = 32; x <= 122; x += 6) {
            const uint8_t col = (((x + shift) & 4) == 0) ? C_WHITE : C_PINK;
            canvas.setPixel(x, ceil_y, col);
        }
    }

    // 5a. Chicken (centered on 10x13 hitbox inside 16x16 sprite, FR-GP-23)
    const int px = static_cast<int>(std::lround(p.world_x - cam_x)) - 3;
    const int py = static_cast<int>(std::lround(p.y)) - 3;
    Sprites::drawChicken(canvas, px, py, chicken, p.anim, p.anim_frame, false, 1);

    // Airborne stamina bar (hidden during unlimited perk flight)
    if (!p.on_ground && !candy_active && session.phase() == game::RunPhase::Playing) {
        const int bar_x = px + 3;
        const int bar_y = py - 2;
        if (bar_y >= 2) {
            canvas.fillRect(bar_x - 1, bar_y - 1, 12, 4, C_BLACK);
            canvas.fillRect(bar_x, bar_y, 10, 2, C_DARK_GREY);
            const int fill = static_cast<int>(std::lround(
                10.0f * (p.stamina / config::K_FLAP_STAMINA)));
            if (fill > 0) {
                const uint8_t scol = (p.stamina > 0.25f * config::K_FLAP_STAMINA) ? C_YELLOW : C_RED;
                canvas.fillRect(bar_x, bar_y, fill, 2, scol);
            }
        }
    }

    // 5b. Particles
    for (std::size_t i = 0; i < world.particles.size(); ++i) {
        const auto& pt = world.particles[i];
        const int sx = static_cast<int>(std::lround(pt.x - cam_x));
        const int sy = static_cast<int>(std::lround(pt.y));
        if (pt.kind == 0) {
            const uint8_t sf = static_cast<uint8_t>(
                std::min<int>(2, static_cast<int>(pt.age / 0.06f)));
            Sprites::drawCoinSparkle(canvas, sx, sy, sf);
        } else if (pt.kind == 1) {
            canvas.fillRect(sx, sy, 2, 2, candy_active ? C_PINK : C_WHITE);
        } else {
            canvas.fillRect(sx, sy, 2, 2, C_LIGHT_GREY);
        }
    }
}

} // namespace schreihuhn::render
