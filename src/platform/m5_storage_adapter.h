#pragma once

#include "schreihuhn/hal/i_storage.h"

namespace schreihuhn::platform {

class M5StorageAdapter : public hal::IStorage {
public:
    bool load(persistence::SaveData& out) override;
    bool save(const persistence::SaveData& in) override;
    bool reset() override;
};

} // namespace schreihuhn::platform
