#pragma once

// Test utility — FakeClock returns a controllable monotonic time.

#include "wildlife/io/iclock.hpp"

namespace wildlife {
namespace test {

class FakeClock : public IClock {
  public:
    explicit FakeClock(uint64_t initial_ms = 0) noexcept : _ms(initial_ms) {}

    uint64_t millis() const noexcept override { return _ms; }

    void advance(uint64_t delta_ms) noexcept { _ms += delta_ms; }
    void set(uint64_t ms) noexcept { _ms = ms; }

  private:
    uint64_t _ms;
};

} // namespace test
} // namespace wildlife
