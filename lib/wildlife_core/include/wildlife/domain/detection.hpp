#pragma once

// Wildlife Core — Detection domain types.
// Pure data; no Arduino dependencies.

#include <cstdint>
#include <string>
#include <vector>

namespace wildlife {

struct BBox {
    int16_t x{0}, y{0}, w{0}, h{0};

    constexpr bool operator==(const BBox& o) const noexcept {
        return x == o.x && y == o.y && w == o.w && h == o.h;
    }
};

struct Detection {
    int32_t class_id{-1};
    uint8_t score{0}; // 0–100
    BBox bbox{};
    uint64_t ts_ms{0}; // monotonic ms (from IClock)
};

struct DeviceMeta {
    std::string device_id;
    std::string fw_version;
    std::string board_id;
    int16_t rssi{0};
    int32_t battery_mv{0};
};

// A snapshot_id is empty when no image is attached to this event.
struct DetectionEvent {
    std::vector<Detection> detections;
    DeviceMeta meta;
    uint64_t event_ts_ms{0};
    std::string snapshot_id;
};

struct JpegBuffer {
    std::vector<uint8_t> data;
};

} // namespace wildlife
