#include "schreihuhn/audio/mic_classifier.h"
#include "schreihuhn/config/tuning.h"
#include "schreihuhn/core/math_types.h"

#include <algorithm>
#include <cmath>

namespace schreihuhn::audio {

void MicClassifier::reset() {
    current_ = MicClassification{};
    calibrating_ = false;
    calib_count_ = 0;
    gate_remaining_ms_ = 0.0f;
    shout_frames_ = 0;
}

void MicClassifier::beginCalibration() {
    calibrating_ = true;
    calib_count_ = 0;
    gate_remaining_ms_ = 0.0f;
    shout_frames_ = 0;
    current_.calibrated = false;
    current_.gated = false;
    current_.state = VoiceState::Still;
    current_.intensity = 0.0f;
}

void MicClassifier::finishCalibration() {
    calibrating_ = false;
    float median = -60.0f;
    if (calib_count_ > 0) {
        std::array<float, kMaxCalibFrames> sorted = calib_samples_;
        std::sort(sorted.begin(), sorted.begin() + static_cast<std::ptrdiff_t>(calib_count_));
        if ((calib_count_ & 1u) != 0u) {
            median = sorted[calib_count_ / 2u];
        } else {
            median = 0.5f * (sorted[calib_count_ / 2u - 1u] + sorted[calib_count_ / 2u]);
        }
    }

    // FR-AU-03, FR-AU-04
    current_.floor_db = core::clampf(median, config::K_FLOOR_MIN_DB, config::K_FLOOR_MAX_DB);
    current_.talk_db  = current_.floor_db + config::K_TALK_OFFSET_DB;
    current_.shout_db = std::max(current_.floor_db + config::K_SHOUT_OFFSET_DB,
                                 config::K_SHOUT_ABS_MIN_DB);
    if (current_.shout_db < current_.talk_db + config::K_SHOUT_MIN_GAP_DB) {
        current_.shout_db = current_.talk_db + config::K_SHOUT_MIN_GAP_DB;
    }

    current_.smoothed_db = current_.floor_db;
    current_.state       = VoiceState::Still;
    current_.intensity   = 0.0f;
    current_.calibrated  = true;
    current_.gated       = false;
    gate_remaining_ms_   = 0.0f;
    shout_frames_        = 0;
}

void MicClassifier::triggerSoundGate(uint16_t sound_duration_ms) {
    const float gate_ms = static_cast<float>(sound_duration_ms) +
                          static_cast<float>(config::K_GATE_TAIL_MS);
    if (gate_ms > gate_remaining_ms_) {
        gate_remaining_ms_ = gate_ms;
    }
    shout_frames_ = 0;
    current_.gated = (gate_remaining_ms_ > 0.0f);
}

void MicClassifier::clearGate() {
    gate_remaining_ms_ = 0.0f;
    current_.gated = false;
}

void MicClassifier::processSamples(const int16_t* samples, std::size_t count, float dt_sec) {
    if (samples == nullptr || count == 0) {
        processFrameDb(-90.0f, dt_sec);
        return;
    }

    double mean = 0.0;
    if constexpr (config::K_DC_FILTER) {
        double sum = 0.0;
        for (std::size_t i = 0; i < count; ++i) {
            sum += static_cast<double>(samples[i]);
        }
        mean = sum / static_cast<double>(count);
    }

    double sum_sq = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        const double v = static_cast<double>(samples[i]) - mean;
        sum_sq += v * v;
    }
    const double rms = std::sqrt(sum_sq / static_cast<double>(count));

    // FR-AU-01
    float raw_db = -90.0f;
    if (rms > 1.0) {
        raw_db = std::max(-90.0f, 20.0f * std::log10(static_cast<float>(rms) / 32768.0f));
    }
    processFrameDb(raw_db, dt_sec);
}

void MicClassifier::processFrameDb(float raw_db, float dt_sec) {
    const float clamped_raw = core::clampf(raw_db, -90.0f, 0.0f);
    current_.raw_db = clamped_raw;

    if (calibrating_) {
        if (calib_count_ < kMaxCalibFrames) {
            calib_samples_[calib_count_++] = clamped_raw;
        }
        return;
    }

    // FR-AU-10: Gating freezes smoothed_db and state (no new SHOUT transition allowed)
    if (gate_remaining_ms_ > 0.0f) {
        gate_remaining_ms_ -= dt_sec * 1000.0f;
        shout_frames_ = 0;
        if (gate_remaining_ms_ > 0.0f) {
            current_.gated = true;
            return;
        }
        gate_remaining_ms_ = 0.0f;
        current_.gated = false;
    }

    // FR-AU-02: Asymmetric EMA smoothing
    const float alpha = (clamped_raw > current_.smoothed_db)
                            ? config::K_SMOOTH_ATTACK
                            : config::K_SMOOTH_RELEASE;
    current_.smoothed_db += alpha * (clamped_raw - current_.smoothed_db);

    if (current_.smoothed_db >= current_.shout_db) {
        if (shout_frames_ < 255) {
            ++shout_frames_;
        }
    } else {
        shout_frames_ = 0;
    }

    // FR-AU-05: Classification with hysteresis
    switch (current_.state) {
        case VoiceState::Still:
            if (shout_frames_ >= config::K_SHOUT_MIN_FRAMES) {
                current_.state = VoiceState::Shout;
            } else if (current_.smoothed_db >= current_.talk_db) {
                current_.state = VoiceState::Talk;
            }
            break;

        case VoiceState::Talk:
            if (shout_frames_ >= config::K_SHOUT_MIN_FRAMES) {
                current_.state = VoiceState::Shout;
            } else if (current_.smoothed_db < current_.talk_db - config::K_HYSTERESIS_DB) {
                current_.state = VoiceState::Still;
            }
            break;

        case VoiceState::Shout:
            if (current_.smoothed_db < current_.shout_db - config::K_HYSTERESIS_DB) {
                if (current_.smoothed_db >= current_.talk_db - config::K_HYSTERESIS_DB) {
                    current_.state = VoiceState::Talk;
                } else {
                    current_.state = VoiceState::Still;
                }
            }
            break;
    }

    // FR-AU-06: TALK intensity in [0, 1]
    if (current_.state == VoiceState::Talk) {
        const float denom = current_.shout_db - current_.talk_db;
        if (denom > 0.01f) {
            current_.intensity = core::clampf((current_.smoothed_db - current_.talk_db) / denom, 0.0f, 1.0f);
        } else {
            current_.intensity = 0.0f;
        }
    } else {
        current_.intensity = 0.0f;
    }
}

} // namespace schreihuhn::audio
