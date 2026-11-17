#include "m5_display_adapter.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/render/palette.h"

#include <utility/M5IOE1_Class.hpp>

#include <algorithm>
#include <cstring>

namespace schreihuhn::platform {

namespace {

void sendCo5300Command(lgfx::IBus* bus, int cs_pin, uint8_t cmd, const uint8_t* args, std::size_t arg_len) {
    if (cs_pin >= 0) {
        lgfx::gpio_lo(cs_pin);
    }
    const uint8_t hdr[4] = {0x02, 0x00, cmd, 0x00};
    for (std::size_t i = 0; i < 4; ++i) {
        bus->writeCommand(hdr[i], 8);
    }
    for (std::size_t i = 0; i < arg_len; ++i) {
        bus->writeCommand(args[i], 8);
    }
    bus->wait();
    if (cs_pin >= 0) {
        lgfx::gpio_hi(cs_pin);
    }
}

void resetAndInitStopWatchAmoled(uint8_t brightness) {
    auto* panel = M5.Display.getPanel();
    auto* bus = panel ? panel->getBus() : nullptr;
    if (!panel || !bus) {
        return;
    }

    const int cs_pin = panel->config().pin_cs;
    if (cs_pin >= 0) {
        lgfx::gpio_hi(cs_pin);
    }

    if (M5.getBoard() == m5::board_t::board_M5StopWatch) {
        auto& ioe1 = M5.getIOExpander(0);
        // Ensure L3B AMOLED power rail (gpio8) and reset lines (gpio4, gpio5) are active outputs.
        ioe1.setHighImpedance(m5::M5IOE1_Class::gpio8, false);
        ioe1.setDirection(m5::M5IOE1_Class::gpio8, true);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio8, true);

        ioe1.setHighImpedance(m5::M5IOE1_Class::gpio4, false);
        ioe1.setHighImpedance(m5::M5IOE1_Class::gpio5, false);
        ioe1.setDirection(m5::M5IOE1_Class::gpio4, true);
        ioe1.setDirection(m5::M5IOE1_Class::gpio5, true);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio4, true);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio5, true);

        // Allow L3B power to settle from 0V on cold boot before asserting a clean hardware reset.
        delay(20);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio4, false);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio5, false);
        delay(15);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio4, true);
        ioe1.digitalWrite(m5::M5IOE1_Class::gpio5, true);
        // M5GFX pmic_ops waits only 2ms after releasing reset; CO5300/CST9217 need longer on cold boot.
        delay(60);

        if (auto* touch = M5.Display.touch()) {
            touch->init();
        }
    }

    // Re-issue full Panel_StopWatch (CO5300) QSPI init inside an explicit SPI transaction so
    // cold-booted displays that missed M5.begin()'s early init come up reliably.
    bus->wait();
    bus->beginTransaction();

    sendCo5300Command(bus, cs_pin, 0x11, nullptr, 0);
    delay(150);

    static constexpr uint8_t kSpiMode[] = {0x80};
    static constexpr uint8_t kColMod[] = {0x55};
    static constexpr uint8_t kTeOn[] = {0x80};
    static constexpr uint8_t kTeLine[] = {0x01, 0xD2};
    static constexpr uint8_t kCtrl[] = {0x20};
    static constexpr uint8_t kMadctl[] = {0x00};

    sendCo5300Command(bus, cs_pin, 0xC4, kSpiMode, sizeof(kSpiMode));
    sendCo5300Command(bus, cs_pin, 0x3A, kColMod, sizeof(kColMod));
    sendCo5300Command(bus, cs_pin, 0x35, kTeOn, sizeof(kTeOn));
    sendCo5300Command(bus, cs_pin, 0x44, kTeLine, sizeof(kTeLine));
    sendCo5300Command(bus, cs_pin, 0x53, kCtrl, sizeof(kCtrl));
    sendCo5300Command(bus, cs_pin, 0x20, nullptr, 0);
    sendCo5300Command(bus, cs_pin, 0x36, kMadctl, sizeof(kMadctl));
    sendCo5300Command(bus, cs_pin, 0x51, &brightness, 1);
    sendCo5300Command(bus, cs_pin, 0x29, nullptr, 0);
    delay(20);

    bus->endTransaction();
}

} // namespace

bool M5DisplayAdapter::init() {
    if (!M5.Display.getPanel() || M5.Display.width() <= 0 || M5.Display.height() <= 0) {
        ready_ = false;
        return false;
    }

    const uint8_t brightness = static_cast<uint8_t>((config::K_BRIGHTNESS * 255u) / 100u);
    resetAndInitStopWatchAmoled(brightness);

    M5.Display.setBrightness(brightness);
    M5.Display.setAutoDisplay(false);

    for (std::size_t i = 0; i < 16; ++i) {
        const uint16_t c = render::K_PALETTE_RGB565[i];
        // Pre-swap bytes for direct write as lgfx::swap565_t (rgb565_2Byte)
        swapped_palette_[i] = static_cast<uint16_t>((c >> 8) | (c << 8));
    }

    offset_x_ = std::max<int32_t>(0, (M5.Display.width() - kScaledWidth) / 2);
    offset_y_ = std::max<int32_t>(0, (M5.Display.height() - kScaledWidth) / 2);

    M5.Display.fillScreen(TFT_BLACK);
    flushDisplay();

    ready_ = true;
    return true;
}

void M5DisplayAdapter::flushDisplay() {
    // M5GFX 0.3.0 Panel_AMOLED_Framebuffer inherits empty beginTransaction/endTransaction
    // from Panel_FrameBufferBase. In Arduino builds, Bus_SPI::endTransaction() overwrites
    // SPI_USER_REG with SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN, breaking subsequent QSPI
    // commands in display() unless beginTransaction/wait/CS-high/endTransaction surround it.
    auto* panel = M5.Display.getPanel();
    auto* bus   = panel ? panel->getBus() : nullptr;

    if (bus) {
        bus->beginTransaction();
    }
    M5.Display.display();
    if (bus) {
        bus->wait();
        const int cs = panel->config().pin_cs;
        if (cs >= 0) {
            lgfx::gpio_hi(cs);
        }
        bus->endTransaction();
    }
}

void M5DisplayAdapter::present(const hal::Canvas155& canvas) {
    if (!ready_) {
        return;
    }

    constexpr int log_sz = hal::Canvas155::kSize; // 155
    constexpr int scale  = config::K_DISPLAY_SCALE; // 3
    constexpr int phy_sz = kScaledWidth;          // 465

    uint16_t* row0 = strip_buf_.data();
    uint16_t* row1 = row0 + phy_sz;
    uint16_t* row2 = row1 + phy_sz;
    const auto* strip_swap565 = reinterpret_cast<const lgfx::swap565_t*>(row0);

    M5.Display.startWrite();
    for (int ly = 0; ly < log_sz; ++ly) {
        const uint8_t* src_row = &canvas.pixels[static_cast<std::size_t>(ly * log_sz)];
        for (int lx = 0; lx < log_sz; ++lx) {
            const uint16_t col = swapped_palette_[src_row[lx] & 0x0Fu];
            const int px = lx * scale;
            row0[px]     = col;
            row0[px + 1] = col;
            row0[px + 2] = col;
        }
        std::memcpy(row1, row0, static_cast<std::size_t>(phy_sz) * sizeof(uint16_t));
        std::memcpy(row2, row0, static_cast<std::size_t>(phy_sz) * sizeof(uint16_t));
        M5.Display.pushImage(offset_x_, offset_y_ + ly * scale, phy_sz, scale, strip_swap565);
    }
    M5.Display.endWrite();

    flushDisplay();
}

} // namespace schreihuhn::platform
