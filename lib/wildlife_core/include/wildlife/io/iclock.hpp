#pragma once
#include <cstdint>

namespace wildlife {

class IClock {
public:
    virtual ~IClock() = default;
    virtual uint64_t millis() const noexcept = 0;
};

} // namespace wildlife
