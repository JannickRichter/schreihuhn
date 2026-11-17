#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace schreihuhn::audio {

enum class VoiceState : uint8_t {
    Still = 0,
    Talk  = 1,
    Shout = 2
};

struct MicClassification {
    VoiceState state      = VoiceState::Still;
    float intensity       = 0.0f;   // 0..1 within TALK (FR-AU-06)
    float raw_db          = -90.0f; // last unsmoothed frame dBFS
    float smoothed_db     = -90.0f; // EMA smoothed dBFS
    float floor_db        = -60.0f; // calibrated noise floor
    float talk_db         = -48.0f; // talk threshold
    float shout_db        = -30.0f; // shout threshold
    bool  calibrated      = false;
    bool  gated           = false;
};

class MicClassifier {
public:
    static constexpr std::size_t kMaxCalibFrames = 96;

    void reset();

    void beginCalibration();
    void finishCalibration();
    bool isCalibrating() const { return calibrating_; }

    void triggerSoundGate(uint16_t sound_duration_ms);
    void clearGate();

    void processSamples(const int16_t* samples, std::size_t count, float dt_sec);
    void processFrameDb(float raw_db, float dt_sec);

    const MicClassification& current() const { return current_; }

private:
    MicClassification current_{};
    bool calibrating_ = false;
    std::array<float, kMaxCalibFrames> calib_samples_{};
    std::size_t calib_count_ = 0;

    float gate_remaining_ms_ = 0.0f;
    uint8_t shout_frames_ = 0;
};

} // namespace schreihuhn::audio
