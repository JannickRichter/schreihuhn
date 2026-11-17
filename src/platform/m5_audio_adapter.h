#pragma once

#include <M5Unified.h>
#include <cstdint>
#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/audio/sound_catalog.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/hal/i_audio_out.h"

namespace schreihuhn::platform {

class M5AudioAdapter : public hal::IAudioOut {
public:
    static constexpr std::size_t kSamplesPerFrame =
        (config::K_MIC_RATE * config::K_MIC_FRAME_MS) / 1000u; // 320 samples

    bool init(audio::MicClassifier& classifier);

    void update(uint32_t now_ms, bool want_mic_active);

    void playSound(audio::SoundId id) override;
    void setSoundEnabled(bool on) override;
    bool isSoundEnabled() const override { return sound_enabled_; }

    bool isMicAvailable() const { return mic_available_; }

private:
    enum class CodecMode : uint8_t {
        Idle          = 0,
        MicActive     = 1,
        SpeakerActive = 2
    };

    void switchToMic();
    void switchToSpeaker();
    void queueBothMicBuffers();

    audio::MicClassifier* classifier_ = nullptr;
    CodecMode mode_ = CodecMode::Idle;
    bool mic_available_ = false;
    bool sound_enabled_ = true;

    audio::SoundSequence active_seq_{};
    uint8_t active_note_idx_ = 0;
    uint32_t note_end_ms_ = 0;
    bool playing_notes_ = false;

    int16_t mic_buf_[2][kSamplesPerFrame]{};
    uint8_t next_buf_idx_ = 0;
    uint8_t mic_discard_frames_ = 0;
};

} // namespace schreihuhn::platform
