#pragma once

// Wildlife Spotter — Core
// SemVer triple parsed from the firmware version string. Pure logic so the
// host coverage harness can exercise it without an MCU SDK.

#include <cstdint>
#include <string_view>

namespace wildlife {

struct SemVer {
    std::uint16_t major{0};
    std::uint16_t minor{0};
    std::uint16_t patch{0};

    constexpr bool operator==(const SemVer& o) const noexcept {
        return major == o.major && minor == o.minor && patch == o.patch;
    }
};

// Parses "MAJOR.MINOR.PATCH" with optional "-suffix". Returns false on any
// malformed input; never throws. `out` is left untouched on failure.
bool parse_semver(std::string_view in, SemVer& out) noexcept;

} // namespace wildlife
