#pragma once

// Wildlife Core — Typed enums for RuntimeConfig categorical fields.
// Every mode/transport/source string is mapped here; the string table lives
// in runtime_enums.cpp.  Use these instead of raw std::string comparisons.

#include <cstdint>
#include <string_view>

namespace wildlife {

// RuntimeConfig::Power::mode
enum class PowerMode : uint8_t {
    AlwaysOn = 0,
    PirDeepSleep,
    Unknown, // catch-all for unrecognised strings
};

// RuntimeConfig::Snapshot::transport
enum class SnapshotTransport : uint8_t {
    MqttChunked = 0,
    HttpUrl,
    Unknown,
};

// RuntimeConfig::Snapshot::source
enum class SnapshotSource : uint8_t {
    Sscma = 0,
    Esp32Cam,
    Null,
    Unknown,
};

// -----------------------------------------------------------------------
// Conversions — guaranteed noexcept; never throw, never return nullptr.
// -----------------------------------------------------------------------

PowerMode power_mode_from_string(std::string_view s) noexcept;
std::string_view power_mode_to_string(PowerMode m) noexcept;

SnapshotTransport snapshot_transport_from_string(std::string_view s) noexcept;
std::string_view snapshot_transport_to_string(SnapshotTransport t) noexcept;

SnapshotSource snapshot_source_from_string(std::string_view s) noexcept;
std::string_view snapshot_source_to_string(SnapshotSource src) noexcept;

} // namespace wildlife