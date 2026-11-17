#include <M5Unified.h>
#include <Arduino.h>
#include <esp_random.h>
#include <cmath>
#include <cstdint>

#include "platform/m5_audio_adapter.h"
#include "platform/m5_display_adapter.h"
#include "platform/m5_haptics_adapter.h"
#include "platform/m5_input_adapter.h"
#include "platform/m5_storage_adapter.h"
#include "schreihuhn/app/app_controller.h"
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/hal/canvas155.h"

namespace {

schreihuhn::platform::M5DisplayAdapter g_display;
schreihuhn::platform::M5AudioAdapter   g_audio;
schreihuhn::platform::M5InputAdapter   g_input;
schreihuhn::platform::M5StorageAdapter g_storage;
schreihuhn::platform::M5HapticsAdapter g_haptics;

schreihuhn::audio::MicClassifier g_mic_classifier;
schreihuhn::app::AppController   g_app;
schreihuhn::hal::Canvas155       g_canvas;

uint32_t g_last_frame_ms = 0;

#ifdef DEBUG_LOG
uint32_t g_dbg_last_ms = 0;
uint32_t g_dbg_frames  = 0;
bool     g_dbg_disp_ok = false;
bool     g_dbg_mic_ok  = false;
bool     g_dbg_boot_printed = false;

const char* voiceStateName(schreihuhn::audio::VoiceState s) {
    switch (s) {
        case schreihuhn::audio::VoiceState::Still: return "STILL";
        case schreihuhn::audio::VoiceState::Talk:  return "TALK";
        case schreihuhn::audio::VoiceState::Shout: return "SHOUT";
    }
    return "STILL";
}
#endif

} // namespace

void setup() {
#ifdef DEBUG_LOG
    Serial.begin(115200);
#endif

    // Allow cold-boot PMIC/oscillator rails to settle before M5Unified I2C probing.
    delay(50);

    auto cfg = M5.config();
    cfg.fallback_board = m5::board_t::board_M5StopWatch;
    M5.begin(cfg);

#ifdef RESET_SAVE
    g_storage.reset();
#endif

    const bool disp_ok = g_display.init();
    const bool mic_ok = g_audio.init(g_mic_classifier);
    g_app.init(g_storage,
               g_audio,
               g_haptics,
               g_mic_classifier,
               mic_ok,
               static_cast<uint32_t>(esp_random()));

    const uint32_t now = millis();
    g_last_frame_ms = now;
#ifdef DEBUG_LOG
    g_dbg_last_ms = now;
    g_dbg_disp_ok = disp_ok;
    g_dbg_mic_ok = mic_ok;
    const uint8_t pm1_src = M5.Power.M5pm1.readRegister8(0x04);
    const uint8_t pm1_wake = M5.Power.M5pm1.readRegister8(0x05);
    const uint8_t pm1_pwr = M5.Power.M5pm1.readRegister8(0x06);
    const uint8_t pm1_hold = M5.Power.M5pm1.readRegister8(0x07);
    const uint8_t pm1_wdt = M5.Power.M5pm1.readRegister8(0x0A);
    const uint8_t ioe1_out_l = M5.getIOExpander(0).readRegister8(0x05);
    const uint8_t ioe1_dir_l = M5.getIOExpander(0).readRegister8(0x03);
    Serial.printf("[DBG] SCHREIHUHN booted board=%d disp=%dx%d disp_ok=%d mic_ok=%d bri=%u psram=%lu free_psram=%lu\n",
                  static_cast<int>(M5.getBoard()),
                  static_cast<int>(M5.Display.width()),
                  static_cast<int>(M5.Display.height()),
                  disp_ok ? 1 : 0,
                  mic_ok ? 1 : 0,
                  static_cast<unsigned>(M5.Display.getBrightness()),
                  static_cast<unsigned long>(ESP.getPsramSize()),
                  static_cast<unsigned long>(ESP.getFreePsram()));
    Serial.printf("[DBG] pwr vbat=%dmV vbus=%dmV lvl=%ld%% chg=%d src=0x%02x wake=0x%02x cfg=0x%02x hold=0x%02x wdt=0x%02x ioe_dir=0x%02x ioe_out=0x%02x\n",
                  static_cast<int>(M5.Power.getBatteryVoltage()),
                  static_cast<int>(M5.Power.getVBUSVoltage()),
                  static_cast<long>(M5.Power.getBatteryLevel()),
                  M5.Power.isCharging() ? 1 : 0,
                  pm1_src,
                  pm1_wake,
                  pm1_pwr,
                  pm1_hold,
                  pm1_wdt,
                  ioe1_dir_l,
                  ioe1_out_l);
#else
    (void)disp_ok;
#endif
}

void loop() {
    const uint32_t frame_start_ms = millis();
    uint32_t dt_ms = frame_start_ms - g_last_frame_ms;
    g_last_frame_ms = frame_start_ms;
    if (dt_ms > 100u) {
        dt_ms = 100u;
    }

    M5.update();

    const auto raw_input = g_input.poll(g_app.state());
    g_audio.update(frame_start_ms, g_app.wantsMicActive());
    g_haptics.update(frame_start_ms);

    g_app.tick(dt_ms, raw_input);
    g_app.render(g_canvas);
    g_display.present(g_canvas);

#ifdef DEBUG_LOG
    ++g_dbg_frames;
    const uint32_t dbg_now = millis();
    const uint32_t dbg_elapsed = dbg_now - g_dbg_last_ms;
    if (dbg_elapsed >= 1000u) {
        if (!g_dbg_boot_printed) {
            g_dbg_boot_printed = true;
            const uint8_t pm1_src = M5.Power.M5pm1.readRegister8(0x04);
            const uint8_t pm1_wake = M5.Power.M5pm1.readRegister8(0x05);
            const uint8_t pm1_pwr = M5.Power.M5pm1.readRegister8(0x06);
            const uint8_t pm1_hold = M5.Power.M5pm1.readRegister8(0x07);
            const uint8_t pm1_wdt = M5.Power.M5pm1.readRegister8(0x0A);
            const uint8_t ioe1_out_l = M5.getIOExpander(0).readRegister8(0x05);
            const uint8_t ioe1_dir_l = M5.getIOExpander(0).readRegister8(0x03);
            Serial.printf("[DBG] hw board=%d disp=%dx%d disp_ok=%d mic_ok=%d bri=%u psram=%lu free_psram=%lu state=%d\n",
                          static_cast<int>(M5.getBoard()),
                          static_cast<int>(M5.Display.width()),
                          static_cast<int>(M5.Display.height()),
                          g_dbg_disp_ok ? 1 : 0,
                          g_dbg_mic_ok ? 1 : 0,
                          static_cast<unsigned>(M5.Display.getBrightness()),
                          static_cast<unsigned long>(ESP.getPsramSize()),
                          static_cast<unsigned long>(ESP.getFreePsram()),
                          static_cast<int>(g_app.state()));
            Serial.printf("[DBG] pwr vbat=%dmV vbus=%dmV lvl=%ld%% chg=%d src=0x%02x wake=0x%02x cfg=0x%02x hold=0x%02x wdt=0x%02x ioe_dir=0x%02x ioe_out=0x%02x\n",
                          static_cast<int>(M5.Power.getBatteryVoltage()),
                          static_cast<int>(M5.Power.getVBUSVoltage()),
                          static_cast<long>(M5.Power.getBatteryLevel()),
                          M5.Power.isCharging() ? 1 : 0,
                          pm1_src,
                          pm1_wake,
                          pm1_pwr,
                          pm1_hold,
                          pm1_wdt,
                          ioe1_dir_l,
                          ioe1_out_l);
        }
        const auto& m = g_mic_classifier.current();
        const unsigned long fps =
            (dbg_elapsed > 0u) ? ((static_cast<unsigned long>(g_dbg_frames) * 1000ul) / dbg_elapsed) : 0ul;
        const long world_x = std::lround(g_app.runSession().player().world_x);
        const unsigned long heap = static_cast<unsigned long>(ESP.getFreeHeap());
        Serial.printf(
            "[DBG] lvl=%.1fdB floor=%.1fdB talk=%.1fdB shout=%.1fdB state=%s fps=%lu x=%ld heap=%lu\n",
            static_cast<double>(m.smoothed_db),
            static_cast<double>(m.floor_db),
            static_cast<double>(m.talk_db),
            static_cast<double>(m.shout_db),
            voiceStateName(m.state),
            fps,
            world_x,
            heap);
        g_dbg_frames = 0;
        g_dbg_last_ms = dbg_now;
    }
#endif

    const uint32_t work_ms = millis() - frame_start_ms;
    if (work_ms < 16u) {
        delay(16u - work_ms);
    } else {
        delay(1);
    }
}
