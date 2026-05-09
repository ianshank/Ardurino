#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace wildlife {

/// Returns true if the given numeric class_id should be processed.
/// If classes_of_interest is empty, all class IDs are accepted.
/// Otherwise only IDs whose decimal string representation appears in the list
/// are accepted.
inline bool matches_class_filter(int32_t class_id,
                                  const std::vector<std::string>& classes_of_interest) noexcept {
    if (classes_of_interest.empty()) return true;
    char buf[12];
    std::snprintf(buf, sizeof(buf), "%d", class_id);
    for (const auto& s : classes_of_interest) {
        if (s == buf) return true;
    }
    return false;
}

} // namespace wildlife
