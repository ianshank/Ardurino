#pragma once

#include "wildlife/io/iclock.hpp"

namespace wildlife {

class EspClock final : public IClock {
  public:
    uint64_t millis() const noexcept override;
};

} // namespace wildlife