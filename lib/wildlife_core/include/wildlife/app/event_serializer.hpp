#pragma once

// Wildlife Core — EventSerializer
// Builds a JSON string for a DetectionEvent directly (no JVal round-trip).
// Schema: detection.v1

#include "wildlife/domain/detection.hpp"

#include <cstdint>
#include <cstdio>
#include <optional>
#include <string>
#include <string_view>

namespace wildlife {
namespace app {

struct SerializerConfig {
    std::string schema_version = "detection.v1";
};

class EventSerializer {
  public:
    explicit EventSerializer(SerializerConfig cfg = {}) noexcept;

    // Serialize a DetectionEvent to a compact JSON string.
    // Optional label resolver: a callable (int32_t class_id) → std::string label.
    // If resolver is null, labels are omitted.
    std::string serialize(const DetectionEvent& event) const noexcept;

    // Serialize with a label resolver callable.
    template <typename LabelFn>
    std::string serialize_with_labels(const DetectionEvent& event, LabelFn&& fn) const noexcept;

    const SerializerConfig& config() const noexcept { return _cfg; }

    // JSON-escape a string value (exposed for testing / reuse).
    static std::string json_escape(std::string_view s) noexcept;

  private:
    SerializerConfig _cfg;

    void append_event_header(std::string& out, const DetectionEvent& ev) const noexcept;
    void append_meta(std::string& out, const DeviceMeta& meta) const noexcept;
};

// ---------------------------------------------------------------------------
// Template implementation
// ---------------------------------------------------------------------------

template <typename LabelFn>
std::string EventSerializer::serialize_with_labels(const DetectionEvent& event,
                                                   LabelFn&& resolver) const noexcept {
    std::string out;
    out.reserve(512);
    out += "{\"schema\":\"";
    out += _cfg.schema_version;
    out += "\",\"event_ts_ms\":";
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(event.event_ts_ms));
    out += buf;
    out += ",\"snapshot_id\":\"";
    out += json_escape(event.snapshot_id);
    out += "\",\"meta\":";
    append_meta(out, event.meta);
    out += ",\"detections\":[";
    bool first = true;
    for (const auto& d : event.detections) {
        if (!first)
            out += ',';
        std::string label = resolver(d.class_id);
        out += "{\"class_id\":";
        std::snprintf(buf, sizeof(buf), "%d", d.class_id);
        out += buf;
        if (!label.empty()) {
            out += ",\"label\":\"";
            out += json_escape(label);
            out += "\"";
        }
        out += ",\"score\":";
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(d.score));
        out += buf;
        out += ",\"bbox\":{\"x\":";
        std::snprintf(buf, sizeof(buf), "%d", d.bbox.x);
        out += buf;
        out += ",\"y\":";
        std::snprintf(buf, sizeof(buf), "%d", d.bbox.y);
        out += buf;
        out += ",\"w\":";
        std::snprintf(buf, sizeof(buf), "%d", d.bbox.w);
        out += buf;
        out += ",\"h\":";
        std::snprintf(buf, sizeof(buf), "%d", d.bbox.h);
        out += buf;
        out += "},\"ts_ms\":";
        std::snprintf(buf, sizeof(buf), "%llu", static_cast<unsigned long long>(d.ts_ms));
        out += buf;
        out += '}';
        first = false;
    }
    out += "]}";
    return out;
}

} // namespace app
} // namespace wildlife
