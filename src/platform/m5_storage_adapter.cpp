#include "m5_storage_adapter.h"

#include <Preferences.h>

namespace schreihuhn::platform {

namespace {
constexpr const char* kNvsNamespace = "schreihuhn";
constexpr const char* kNvsKeySave   = "save";
} // namespace

bool M5StorageAdapter::load(persistence::SaveData& out) {
    Preferences prefs;
    if (!prefs.begin(kNvsNamespace, true)) {
        return false;
    }
    const size_t len = prefs.getBytesLength(kNvsKeySave);
    if (len != sizeof(persistence::SaveData)) {
        prefs.end();
        return false;
    }
    persistence::SaveData tmp{};
    const size_t read_len = prefs.getBytes(kNvsKeySave, &tmp, sizeof(tmp));
    prefs.end();
    if (read_len != sizeof(tmp)) {
        return false;
    }
    tmp.sanitize();
    out = tmp;
    return true;
}

bool M5StorageAdapter::save(const persistence::SaveData& in) {
    Preferences prefs;
    if (!prefs.begin(kNvsNamespace, false)) {
        return false;
    }
    persistence::SaveData clean = in;
    clean.sanitize();
    const size_t written = prefs.putBytes(kNvsKeySave, &clean, sizeof(clean));
    prefs.end();
    return written == sizeof(clean);
}

bool M5StorageAdapter::reset() {
    Preferences prefs;
    if (!prefs.begin(kNvsNamespace, false)) {
        return false;
    }
    const bool ok = prefs.clear();
    prefs.end();
    return ok;
}

} // namespace schreihuhn::platform
