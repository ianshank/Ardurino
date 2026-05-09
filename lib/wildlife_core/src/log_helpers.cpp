#include "wildlife/app/log_helpers.hpp"
#include <cctype>

namespace wildlife {

namespace {
    // tolower a single ASCII char, constexpr-safe
    constexpr char ascii_lower(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    bool iequal(std::string_view a, std::string_view b) noexcept {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (ascii_lower(a[i]) != ascii_lower(b[i])) return false;
        }
        return true;
    }
} // namespace

LogLevel parse_log_level(std::string_view s) noexcept {
    if (iequal(s, "debug")) return LogLevel::Debug;
    if (iequal(s, "warn"))  return LogLevel::Warn;
    if (iequal(s, "error")) return LogLevel::Error;
    return LogLevel::Info; // default: info
}

std::string_view log_level_name(LogLevel level) noexcept {
    switch (level) {
        case LogLevel::Debug: return "debug";
        case LogLevel::Info:  return "info";
        case LogLevel::Warn:  return "warn";
        case LogLevel::Error: return "error";
    }
    return "info";
}

} // namespace wildlife
