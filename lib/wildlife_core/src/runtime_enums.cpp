#include "wildlife/domain/runtime_enums.hpp"

namespace wildlife {

// ---------------------------------------------------------------------------
// PowerMode
// ---------------------------------------------------------------------------

PowerMode power_mode_from_string(std::string_view s) noexcept {
    if (s == "pir_deep_sleep") return PowerMode::PirDeepSleep;
    if (s == "always_on")      return PowerMode::AlwaysOn;
    return PowerMode::Unknown;
}

std::string_view power_mode_to_string(PowerMode m) noexcept {
    switch (m) {
        case PowerMode::AlwaysOn:     return "always_on";
        case PowerMode::PirDeepSleep: return "pir_deep_sleep";
        case PowerMode::Unknown:      return "unknown";
    }
    return "unknown";
}

// ---------------------------------------------------------------------------
// SnapshotTransport
// ---------------------------------------------------------------------------

SnapshotTransport snapshot_transport_from_string(std::string_view s) noexcept {
    if (s == "mqtt_chunked") return SnapshotTransport::MqttChunked;
    if (s == "http_url")     return SnapshotTransport::HttpUrl;
    return SnapshotTransport::Unknown;
}

std::string_view snapshot_transport_to_string(SnapshotTransport t) noexcept {
    switch (t) {
        case SnapshotTransport::MqttChunked: return "mqtt_chunked";
        case SnapshotTransport::HttpUrl:     return "http_url";
        case SnapshotTransport::Unknown:     return "unknown";
    }
    return "unknown";
}

// ---------------------------------------------------------------------------
// SnapshotSource
// ---------------------------------------------------------------------------

SnapshotSource snapshot_source_from_string(std::string_view s) noexcept {
    if (s == "sscma")    return SnapshotSource::Sscma;
    if (s == "esp32cam") return SnapshotSource::Esp32Cam;
    if (s == "null")     return SnapshotSource::Null;
    return SnapshotSource::Unknown;
}

std::string_view snapshot_source_to_string(SnapshotSource src) noexcept {
    switch (src) {
        case SnapshotSource::Sscma:    return "sscma";
        case SnapshotSource::Esp32Cam: return "esp32cam";
        case SnapshotSource::Null:     return "null";
        case SnapshotSource::Unknown:  return "unknown";
    }
    return "unknown";
}

} // namespace wildlife
