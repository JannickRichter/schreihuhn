#pragma once

#include "schreihuhn/audio/sound_catalog.h"

namespace schreihuhn::hal {

class IAudioOut {
public:
    virtual ~IAudioOut() = default;
    virtual void playSound(audio::SoundId id) = 0;
    virtual void setSoundEnabled(bool on) = 0;
    virtual bool isSoundEnabled() const = 0;
};

} // namespace schreihuhn::hal
