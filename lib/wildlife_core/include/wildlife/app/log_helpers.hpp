#pragma once

// Wildlife Core — log_helpers
// Shared utilities for converting between LogLevel and string.
// Used by SerialLogger (min-level wiring) and config-driven level selection.

#include "wildlife/io/ilogger.hpp"

#include <cstdio>
#include <string>
#include <string_view>
#include <utility>

namespace wildlife {

// Parse a case-insensitive level string ("debug", "info", "warn", "error").
// Returns LogLevel::Info for unrecognised strings.
LogLevel parse_log_level(std::string_view s) noexcept;

// Return the canonical lowercase name for a level.
std::string_view log_level_name(LogLevel level) noexcept;

namespace detail {
// Convert various value types to key=value formatted strings.
inline std::string to_kv_str(std::string_view v) {
    std::string r;
    r.reserve(v.size());
    for (char c : v)
        r += (c == ' ') ? '_' : c;
    return r;
}
inline std::string to_kv_str(const std::string& v) {
    return to_kv_str(std::string_view{v});
}
inline std::string to_kv_str(const char* v) {
    return to_kv_str(std::string_view{v ? v : ""});
}
inline std::string to_kv_str(bool v) {
    return v ? "true" : "false";
}
inline std::string to_kv_str(int v) {
    char b[16];
    std::snprintf(b, sizeof(b), "%d", v);
    return b;
}
inline std::string to_kv_str(unsigned int v) {
    char b[16];
    std::snprintf(b, sizeof(b), "%u", v);
    return b;
}
inline std::string to_kv_str(long v) {
    char b[24];
    std::snprintf(b, sizeof(b), "%ld", v);
    return b;
}
inline std::string to_kv_str(unsigned long v) {
    char b[24];
    std::snprintf(b, sizeof(b), "%lu", v);
    return b;
}
inline std::string to_kv_str(float v) {
    char b[32];
    std::snprintf(b, sizeof(b), "%.4g", static_cast<double>(v));
    return b;
}
inline std::string to_kv_str(double v) {
    char b[32];
    std::snprintf(b, sizeof(b), "%.4g", v);
    return b;
}
} // namespace detail

/// Build a space-separated "key=value key2=value2 ..." string.
/// Spaces within string values are replaced with underscores.
/// Usage: format_kv("class", 3, "score", 0.85f)
inline std::string format_kv() noexcept {
    return {};
}

template <typename K, typename V, typename... Rest>
std::string format_kv(K&& key, V&& val, Rest&&... rest) {
    std::string r;
    r += detail::to_kv_str(std::forward<K>(key));
    r += '=';
    r += detail::to_kv_str(std::forward<V>(val));
    if constexpr (sizeof...(Rest) > 0) {
        r += ' ';
        r += format_kv(std::forward<Rest>(rest)...);
    }
    return r;
}

} // namespace wildlife
