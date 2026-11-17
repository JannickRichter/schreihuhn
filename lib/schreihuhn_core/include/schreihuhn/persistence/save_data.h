#pragma once

#include <cstdint>
#include "schreihuhn/config/tuning.h"

namespace schreihuhn::persistence {

enum class ChickenId : uint8_t {
    Henni  = 0,
    Goldie = 1,
    Ninja  = 2,
    Count  = 3
};

struct SaveData {
    uint8_t  schema_version = config::K_SAVE_SCHEMA;
    uint32_t coins          = 0;
    uint32_t highscore      = 0;
    uint8_t  unlocked       = 0b001; // Bit 0=HENNI, Bit 1=GOLDIE, Bit 2=NINJA
    uint8_t  selected       = 0;     // 0..2
    bool     sound_on       = true;

    // Validates all fields, resets out-of-range fields to defaults (FR-PS-02),
    // keeps known fields across schema versions (FR-PS-03), forces Bit 0 of unlocked,
    // and ensures selected is unlocked.
    void sanitize() {
        schema_version = config::K_SAVE_SCHEMA;
        if (coins > config::K_COINS_MAX) {
            coins = 0;
        }
        if (highscore > config::K_COINS_MAX) {
            highscore = 0;
        }
        unlocked = static_cast<uint8_t>((unlocked & 0x07u) | 0x01u);
        if (selected >= static_cast<uint8_t>(ChickenId::Count) ||
            (unlocked & static_cast<uint8_t>(1u << selected)) == 0u) {
            selected = 0;
        }
    }

    bool isUnlocked(ChickenId id) const {
        const uint8_t idx = static_cast<uint8_t>(id);
        if (idx >= static_cast<uint8_t>(ChickenId::Count)) {
            return false;
        }
        return (unlocked & static_cast<uint8_t>(1u << idx)) != 0u;
    }

    void setUnlocked(ChickenId id) {
        const uint8_t idx = static_cast<uint8_t>(id);
        if (idx < static_cast<uint8_t>(ChickenId::Count)) {
            unlocked = static_cast<uint8_t>(unlocked | (1u << idx));
        }
    }
};

} // namespace schreihuhn::persistence
