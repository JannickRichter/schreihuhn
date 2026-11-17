#pragma once

#include <cstdint>
#include <vector>
#include "schreihuhn/hal/i_audio_out.h"
#include "schreihuhn/hal/i_haptics.h"
#include "schreihuhn/hal/i_storage.h"

namespace schreihuhn::test {

class MockStorage : public hal::IStorage {
public:
    bool has_data = false;
    bool fail_load = false;
    bool fail_save = false;
    persistence::SaveData stored{};
    int load_calls = 0;
    int save_calls = 0;
    int reset_calls = 0;

    bool load(persistence::SaveData& out) override {
        ++load_calls;
        if (fail_load || !has_data) {
            return false;
        }
        out = stored;
        return true;
    }

    bool save(const persistence::SaveData& in) override {
        ++save_calls;
        if (fail_save) {
            return false;
        }
        stored = in;
        has_data = true;
        return true;
    }

    bool reset() override {
        ++reset_calls;
        has_data = false;
        stored = persistence::SaveData{};
        return true;
    }
};

class MockAudioOut : public hal::IAudioOut {
public:
    bool sound_enabled = true;
    std::vector<audio::SoundId> played_sounds;

    void playSound(audio::SoundId id) override {
        if (!sound_enabled) {
            return;
        }
        played_sounds.push_back(id);
    }

    void setSoundEnabled(bool on) override {
        sound_enabled = on;
    }

    bool isSoundEnabled() const override {
        return sound_enabled;
    }
};

class MockHaptics : public hal::IHaptics {
public:
    std::vector<uint16_t> vibrations;

    void vibrate(uint16_t duration_ms) override {
        vibrations.push_back(duration_ms);
    }
};

} // namespace schreihuhn::test
