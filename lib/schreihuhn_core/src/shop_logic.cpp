#include "schreihuhn/persistence/shop_logic.h"
#include "schreihuhn/config/tuning.h"

namespace schreihuhn::persistence {

uint32_t ShopLogic::priceOf(ChickenId id) {
    switch (id) {
        case ChickenId::Henni:  return 0;
        case ChickenId::Goldie: return config::K_PRICE_GOLDIE;
        case ChickenId::Ninja:  return config::K_PRICE_NINJA;
        default:                return 0;
    }
}

const char* ShopLogic::nameOf(ChickenId id) {
    switch (id) {
        case ChickenId::Henni:  return "HENNI";
        case ChickenId::Goldie: return "GOLDIE";
        case ChickenId::Ninja:  return "NINJA";
        default:                return "HENNI";
    }
}

bool ShopLogic::canAfford(const SaveData& save, ChickenId id) {
    if (id >= ChickenId::Count || save.isUnlocked(id)) {
        return false;
    }
    return save.coins >= priceOf(id);
}

uint32_t ShopLogic::missingCoins(const SaveData& save, ChickenId id) {
    const uint32_t price = priceOf(id);
    if (save.coins >= price) {
        return 0;
    }
    return price - save.coins;
}

bool ShopLogic::buy(SaveData& save, ChickenId id, hal::IStorage& storage) {
    if (!canAfford(save, id)) {
        return false;
    }
    const SaveData backup = save;
    save.coins -= priceOf(id);
    save.setUnlocked(id);
    save.selected = static_cast<uint8_t>(id);
    save.sanitize();

    if (!storage.save(save)) {
        save = backup;
        return false;
    }
    return true;
}

} // namespace schreihuhn::persistence
