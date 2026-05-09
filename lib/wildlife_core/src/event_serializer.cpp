#include "wildlife/app/event_serializer.hpp"

#include <cstdio>

namespace wildlife {
namespace app {

EventSerializer::EventSerializer(SerializerConfig cfg) noexcept : _cfg(std::move(cfg)) {}

std::string EventSerializer::json_escape(std::string_view s) noexcept {
    std::string out;
    out.reserve(s.size() + 4);
    for (unsigned char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += static_cast<char>(c);
            }
        }
    }
    return out;
}

void EventSerializer::append_meta(std::string& out, const DeviceMeta& meta) const noexcept {
    out += "{\"device_id\":\"";
    out += json_escape(meta.device_id);
    out += "\",\"fw_version\":\"";
    out += json_escape(meta.fw_version);
    out += "\",\"board_id\":\"";
    out += json_escape(meta.board_id);
    out += "\",\"rssi\":";
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%d", static_cast<int>(meta.rssi));
    out += buf;
    out += ",\"battery_mv\":";
    std::snprintf(buf, sizeof(buf), "%d", meta.battery_mv);
    out += buf;
    out += "}";
}

std::string EventSerializer::serialize(const DetectionEvent& event) const noexcept {
    // Serialize without labels (class_id only).
    return serialize_with_labels(event, [](int32_t) -> std::string { return ""; });
}

} // namespace app
} // namespace wildlife
