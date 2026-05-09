#include "wildlife/semver.hpp"

namespace wildlife {

namespace {

bool parse_uint(std::string_view s, std::uint16_t& out) noexcept {
    if (s.empty())
        return false;
    std::uint32_t v = 0;
    for (char c : s) {
        if (c < '0' || c > '9')
            return false;
        v = v * 10 + static_cast<std::uint32_t>(c - '0');
        if (v > 0xFFFF)
            return false;
    }
    out = static_cast<std::uint16_t>(v);
    return true;
}

} // namespace

bool parse_semver(std::string_view in, SemVer& out) noexcept {
    if (in.empty())
        return false;

    // Strip optional pre-release suffix introduced by '-'.
    if (auto dash = in.find('-'); dash != std::string_view::npos) {
        in = in.substr(0, dash);
    }

    const auto first = in.find('.');
    if (first == std::string_view::npos)
        return false;
    const auto second = in.find('.', first + 1);
    if (second == std::string_view::npos)
        return false;

    SemVer tmp{};
    if (!parse_uint(in.substr(0, first), tmp.major))
        return false;
    if (!parse_uint(in.substr(first + 1, second - first - 1), tmp.minor))
        return false;
    if (!parse_uint(in.substr(second + 1), tmp.patch))
        return false;

    out = tmp;
    return true;
}

} // namespace wildlife
