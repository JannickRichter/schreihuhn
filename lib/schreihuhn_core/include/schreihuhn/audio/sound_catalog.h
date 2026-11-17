#pragma once

#include <array>
#include <cstdint>

namespace schreihuhn::audio {

enum class SoundId : uint8_t {
    Start    = 0, // SND-START: C5-E5-G5 (100ms) + G5 (150ms) = 450 ms
    Click    = 1, // SND-CLICK: 1500 Hz, 15 ms
    Coin     = 2, // SND-COIN: B5 40 ms -> E6 70 ms = 110 ms
    Buy      = 3, // SND-BUY: C5-E5-G5-C6 (60ms each) = 240 ms
    Error    = 4, // SND-ERROR: 200 Hz, 120 ms
    GameOver = 5, // SND-GAMEOVER: G4-E4-C4 (120ms) + C3 (200ms) = 560 ms
    Hiscore  = 6, // SND-HISCORE: C5-C5-G5 (80ms each) = 240 ms
    Toggle   = 7, // SND-TOGGLE: 1000 Hz, 40 ms
    Perk     = 8, // SND-PERK: E5-G5-B5-E6 (50ms each) = 200 ms
    Count    = 9
};

struct ToneNote {
    uint16_t freq_hz;
    uint16_t duration_ms;
};

struct SoundSequence {
    std::array<ToneNote, 4> notes;
    uint8_t count;
    uint16_t total_duration_ms;
};

inline constexpr SoundSequence getSoundSequence(SoundId id) {
    switch (id) {
        case SoundId::Start:
            return {{{{523, 100}, {659, 100}, {784, 100}, {784, 150}}}, 4, 450};
        case SoundId::Click:
            return {{{{1500, 15}, {0, 0}, {0, 0}, {0, 0}}}, 1, 15};
        case SoundId::Coin:
            return {{{{988, 40}, {1319, 70}, {0, 0}, {0, 0}}}, 2, 110};
        case SoundId::Buy:
            return {{{{523, 60}, {659, 60}, {784, 60}, {1047, 60}}}, 4, 240};
        case SoundId::Error:
            return {{{{200, 120}, {0, 0}, {0, 0}, {0, 0}}}, 1, 120};
        case SoundId::GameOver:
            return {{{{392, 120}, {330, 120}, {262, 120}, {131, 200}}}, 4, 560};
        case SoundId::Hiscore:
            return {{{{523, 80}, {523, 80}, {784, 80}, {0, 0}}}, 3, 240};
        case SoundId::Toggle:
            return {{{{1000, 40}, {0, 0}, {0, 0}, {0, 0}}}, 1, 40};
        case SoundId::Perk:
            return {{{{659, 50}, {784, 50}, {988, 50}, {1319, 50}}}, 4, 200};
        default:
            return {{{{0, 0}, {0, 0}, {0, 0}, {0, 0}}}, 0, 0};
    }
}

} // namespace schreihuhn::audio
