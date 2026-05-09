#include "wildlife/app/event_debouncer.hpp"

#include <algorithm>
#include <cstdio>

namespace wildlife {
namespace app {

EventDebouncer::EventDebouncer(DebouncerConfig cfg, ILogger* log) noexcept
    : _cfg(std::move(cfg)), _log(log) {}

bool EventDebouncer::in_allow_list(const std::string& label) const noexcept {
    if (_cfg.classes_of_interest.empty())
        return true;
    for (const auto& c : _cfg.classes_of_interest) {
        if (c == label)
            return true;
    }
    return false;
}

bool EventDebouncer::should_emit(int32_t class_id, const std::string& label,
                                 uint64_t now_ms) noexcept {
    if (!in_allow_list(label)) {
        if (_log) {
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Debounce: class_id=%d label='%s' not in allow list",
                          class_id, label.c_str());
            _log->debug(buf);
        }
        return false;
    }
    auto it = _last_emit.find(class_id);
    if (it != _last_emit.end()) {
        if ((now_ms - it->second) < _cfg.debounce_ms) {
            if (_log) {
                char buf[64];
                std::snprintf(buf, sizeof(buf), "Debounce: class_id=%d suppressed (window %ums)",
                              class_id, _cfg.debounce_ms);
                _log->debug(buf);
            }
            return false;
        }
        it->second = now_ms;
    } else {
        _last_emit.emplace(class_id, now_ms);
    }
    return true;
}

void EventDebouncer::reset() noexcept {
    _last_emit.clear();
    if (_log)
        _log->debug("Debounce: state reset");
}

} // namespace app
} // namespace wildlife
