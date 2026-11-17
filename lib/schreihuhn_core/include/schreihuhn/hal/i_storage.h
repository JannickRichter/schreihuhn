#pragma once

#include "schreihuhn/persistence/save_data.h"

namespace schreihuhn::hal {

class IStorage {
public:
    virtual ~IStorage() = default;
    virtual bool load(persistence::SaveData& out) = 0;
    virtual bool save(const persistence::SaveData& in) = 0;
    virtual bool reset() = 0;
};

} // namespace schreihuhn::hal
