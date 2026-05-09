#pragma once

#include "wildlife/io/ilogger.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace wildlife {
namespace app {

struct DebouncerConfig {
    uint32_t debounce_ms = 5000;
    // Allow-list of resolved label strings (e.g. "dog", "cat"). The debouncer
    // matches the label produced by the caller's resolver, NOT the raw class_id.
    // If empty, all labels are permitted.
    std::vector<std::string> classes_of_interest;
};

class EventDebouncer {
  public:
    // Optional logger for suppressed-event debug output.
    explicit EventDebouncer(DebouncerConfig cfg = {}, ILogger* log = nullptr) noexcept;

    // Returns true if this (class_id, label) detection should be emitted.
    // Updates the last-emit timestamp on success.
    bool should_emit(int32_t class_id, const std::string& label, uint64_t now_ms) noexcept;

    // Reset all per-class timestamps (e.g. after wake from sleep).
    void reset() noexcept;

    const DebouncerConfig& config() const noexcept { return _cfg; }

  private:
    DebouncerConfig _cfg;
    ILogger* _log{nullptr};
    std::unordered_map<int32_t, uint64_t> _last_emit;

    bool in_allow_list(const std::string& label) const noexcept;
};

} // namespace app
} // namespace wildlife
