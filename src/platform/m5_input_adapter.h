#pragma once

#include <M5Unified.h>
#include "schreihuhn/app/app_controller.h"
#include "schreihuhn/hal/platform_input.h"

namespace schreihuhn::platform {

class M5InputAdapter {
public:
    hal::PlatformInput poll(app::AppState app_state) const;
};

} // namespace schreihuhn::platform
