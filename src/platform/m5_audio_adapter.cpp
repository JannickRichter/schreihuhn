#include "m5_audio_adapter.h"

namespace schreihuhn::platform {

bool M5AudioAdapter::init(audio::MicClassifier& classifier) {
    classifier_ = &classifier;

    auto mic_cfg = M5.Mic.config();
    mic_cfg.sample_rate   = config::K_MIC_RATE;
    mic_cfg.magnification = config::K_MIC_GAIN;
    M5.Mic.config(mic_cfg);

    auto spk_cfg = M5.Speaker.config();
    M5.Speaker.config(spk_cfg);

    // Probe microphone availability once at boot (UC-01 1a)
    if (M5.Speaker.isRunning()) {
        M5.Speaker.end();
    }
    mic_available_ = M5.Mic.begin();
    if (mic_available_) {
        M5.Mic.end();
    }

    // Start in speaker mode for Splash jingle & UI clicks
    switchToSpeaker();
    return mic_available_;
}

void M5AudioAdapter::switchToMic() {
    if (!mic_available_ || mode_ == CodecMode::MicActive) {
        return;
    }
    if (mode_ == CodecMode::SpeakerActive) {
        M5.Speaker.stop();
        M5.Speaker.end();
    }
    if (M5.Mic.begin()) {
        mode_ = CodecMode::MicActive;
        // Discard initial ES8311 ADC startup transient frames after speaker->mic switchover
        mic_discard_frames_ = 3;
        queueBothMicBuffers();
    } else {
        mode_ = CodecMode::Idle;
    }
}

void M5AudioAdapter::switchToSpeaker() {
    if (mode_ == CodecMode::SpeakerActive) {
        return;
    }
    if (mode_ == CodecMode::MicActive) {
        M5.Mic.end();
    }
    if (M5.Speaker.begin()) {
        const uint8_t vol = static_cast<uint8_t>((config::K_VOLUME * 255u) / 100u);
        M5.Speaker.setVolume(vol);
        mode_ = CodecMode::SpeakerActive;
    } else {
        mode_ = CodecMode::Idle;
    }
}

void M5AudioAdapter::queueBothMicBuffers() {
    next_buf_idx_ = 0;
    M5.Mic.record(mic_buf_[0], kSamplesPerFrame, config::K_MIC_RATE);
    M5.Mic.record(mic_buf_[1], kSamplesPerFrame, config::K_MIC_RATE);
}

void M5AudioAdapter::setSoundEnabled(bool on) {
    sound_enabled_ = on;
    if (!sound_enabled_ && playing_notes_) {
        playing_notes_ = false;
        if (mode_ == CodecMode::SpeakerActive) {
            M5.Speaker.stop();
        }
    }
}

void M5AudioAdapter::playSound(audio::SoundId id) {
    if (!sound_enabled_) {
        return;
    }
    const auto seq = audio::getSoundSequence(id);
    if (seq.count == 0) {
        return;
    }

    if (classifier_ != nullptr) {
        // Mic is paused in SpeakerActive; gate only for K_GATE_TAIL_MS after Mic resumes (FR-AU-10/11)
        classifier_->triggerSoundGate(0u);
    }

    switchToSpeaker();
    if (mode_ != CodecMode::SpeakerActive) {
        return;
    }

    active_seq_      = seq;
    active_note_idx_ = 0;
    playing_notes_   = true;
    note_end_ms_     = millis() + active_seq_.notes[0].duration_ms;
    M5.Speaker.tone(static_cast<float>(active_seq_.notes[0].freq_hz),
                    active_seq_.notes[0].duration_ms);
}

void M5AudioAdapter::update(uint32_t now_ms, bool want_mic_active) {
    if (playing_notes_) {
        if (static_cast<int32_t>(now_ms - note_end_ms_) >= 0) {
            ++active_note_idx_;
            if (active_note_idx_ < active_seq_.count) {
                const auto& n = active_seq_.notes[active_note_idx_];
                note_end_ms_ = now_ms + n.duration_ms;
                M5.Speaker.tone(static_cast<float>(n.freq_hz), n.duration_ms);
            } else {
                playing_notes_ = false;
            }
        }
    }

    if (!playing_notes_) {
        if (want_mic_active && mic_available_) {
            if (mode_ != CodecMode::MicActive) {
                switchToMic();
            }
        } else {
            if (mode_ == CodecMode::MicActive) {
                switchToSpeaker();
            }
        }
    }

    if (mode_ == CodecMode::MicActive && classifier_ != nullptr) {
        constexpr float frame_dt = static_cast<float>(config::K_MIC_FRAME_MS) * 0.001f;
        // Drain completed double-buffered mic blocks without blocking (FR-AU-08)
        int guard = 0;
        while (M5.Mic.isRecording() < 2 && guard < 4) {
            if (mic_discard_frames_ > 0) {
                --mic_discard_frames_;
                classifier_->processFrameDb(-90.0f, frame_dt);
            } else {
                classifier_->processSamples(mic_buf_[next_buf_idx_], kSamplesPerFrame, frame_dt);
            }
            M5.Mic.record(mic_buf_[next_buf_idx_], kSamplesPerFrame, config::K_MIC_RATE);
            next_buf_idx_ ^= 1u;
            ++guard;
        }
    }
}

} // namespace schreihuhn::platform
