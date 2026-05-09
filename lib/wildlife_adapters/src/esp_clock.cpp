#include "wildlife/adapters/esp_clock.hpp"

#include <Arduino.h>

namespace wildlife {

uint64_t EspClock::millis() const noexcept {
    return static_cast<uint64_t>(::millis());
}

} // namespace wildlife