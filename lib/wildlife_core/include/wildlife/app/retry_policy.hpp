#pragma once

#include <cstdint>

namespace wildlife {
namespace app {

// Exponential backoff with full jitter.
// delay_ms(attempt) = base_ms * 2^attempt, capped at max_ms, with ±jitter%.
// Deterministic seeding: caller passes a 32-bit entropy value to delay_ms().
struct RetryConfig {
    uint32_t base_ms       = 1000;
    uint32_t max_ms        = 30000;
    uint8_t  max_attempts  = 10;
    uint8_t  jitter_pct    = 25;   // 0–50; applied symmetrically: ±jitter_pct%
};

class RetryPolicy {
public:
    explicit RetryPolicy(RetryConfig cfg = {}) noexcept;

    // Returns backoff delay in ms for the given attempt (0-indexed).
    // entropy is used for jitter — pass a timestamp or free-running counter.
    uint32_t delay_ms(uint32_t attempt, uint32_t entropy = 0) const noexcept;

    // True if the attempt count is within the policy limit.
    bool should_retry(uint32_t attempt) const noexcept;

    const RetryConfig& config() const noexcept { return _cfg; }

private:
    RetryConfig _cfg;
};

} // namespace app
} // namespace wildlife
