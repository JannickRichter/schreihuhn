#pragma once

#include <cstdint>
#include "schreihuhn/hal/i_storage.h"
#include "schreihuhn/persistence/save_data.h"

namespace schreihuhn::persistence {

class ShopLogic {
public:
    static uint32_t priceOf(ChickenId id);
    static const char* nameOf(ChickenId id);
    static bool canAfford(const SaveData& save, ChickenId id);
    static uint32_t missingCoins(const SaveData& save, ChickenId id);
    static bool buy(SaveData& save, ChickenId id, hal::IStorage& storage);
};

} // namespace schreihuhn::persistence
