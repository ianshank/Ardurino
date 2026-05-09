#include "wildlife/app/retry_policy.hpp"
#include <algorithm>

namespace wildlife {
namespace app {

RetryPolicy::RetryPolicy(RetryConfig cfg) noexcept : _cfg(cfg) {}

uint32_t RetryPolicy::delay_ms(uint32_t attempt, uint32_t entropy) const noexcept {
    // Cap attempt to prevent overflow in the shift.
    constexpr uint32_t kMaxShift = 15;
    const uint32_t shift = std::min(attempt, kMaxShift);
    // Base exponential: base_ms * 2^attempt, capped at max_ms.
    uint64_t base = static_cast<uint64_t>(_cfg.base_ms) << shift;
    if (base > _cfg.max_ms) base = _cfg.max_ms;

    // Jitter: ±jitter_pct% using entropy as a cheap pseudo-random source.
    // jitter_range = base * jitter_pct / 100
    // offset = [0, 2*jitter_range] mapped from entropy, then shifted to [-jitter_range, +jitter_range]
    if (_cfg.jitter_pct == 0) return static_cast<uint32_t>(base);
    const uint32_t jitter_range = static_cast<uint32_t>(base * _cfg.jitter_pct / 100u);
    const uint32_t span         = jitter_range * 2 + 1;
    const uint32_t offset       = (span > 0) ? (entropy % span) : 0;
    // Result is in range [base - jitter_range, base + jitter_range].
    int64_t result = static_cast<int64_t>(base)
                   - static_cast<int64_t>(jitter_range)
                   + static_cast<int64_t>(offset);
    if (result < 0) result = 0;
    if (result > static_cast<int64_t>(_cfg.max_ms)) result = _cfg.max_ms;
    return static_cast<uint32_t>(result);
}

bool RetryPolicy::should_retry(uint32_t attempt) const noexcept {
    return attempt < static_cast<uint32_t>(_cfg.max_attempts);
}

} // namespace app
} // namespace wildlife
