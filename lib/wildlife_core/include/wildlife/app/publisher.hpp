#pragma once

// Wildlife Core — Publisher
// Composes EventDebouncer → EventSerializer → SnapshotChunker → ITransport.
// Drives the full outbound pipeline from a raw DetectionEvent + optional JPEG.
// All topics are derived from RuntimeConfig::Mqtt and device_id; no strings are hardcoded.

#include "wildlife/app/event_debouncer.hpp"
#include "wildlife/app/event_serializer.hpp"
#include "wildlife/app/snapshot_chunker.hpp"
#include "wildlife/domain/detection.hpp"
#include "wildlife/domain/runtime_config.hpp"
#include "wildlife/io/ilogger.hpp"
#include "wildlife/io/ipower_transport.hpp"

#include <optional>
#include <string>

namespace wildlife {
namespace app {

enum class PublishOutcome : uint8_t {
    Ok = 0,
    Suppressed,     // debouncer filtered the event
    TransportError, // publish call failed
    NotConnected,   // transport not connected
};

class Publisher {
  public:
    // device_id is mandatory — it segments MQTT topics as
    //   <base_topic>/<device_id>/event  and  <base_topic>/<device_id>/snapshot
    // An empty device_id is logged as a warning but not fatal.
    Publisher(EventDebouncer debouncer, EventSerializer serializer, std::string device_id,
              const RuntimeConfig::Mqtt& mqtt_cfg, ITransport& transport,
              ILogger* log = nullptr) noexcept;

    // Publish a detection event, optionally attaching a JPEG snapshot.
    // snapshot must be non-null if a jpeg is available; pass {} to skip.
    PublishOutcome publish(const DetectionEvent& event, const std::string& resolved_label,
                           std::optional<JpegBuffer> snapshot) noexcept;

    // Overload with a label resolver callable (int32_t → std::string).
    template <typename LabelFn>
    PublishOutcome publish_with_labels(const DetectionEvent& event, LabelFn&& resolver,
                                       std::optional<JpegBuffer> snapshot) noexcept;

    // Non-blocking transport pump.  Call every loop iteration.
    void loop() noexcept { _transport->loop(); }

    const std::string& device_id() const noexcept { return _device_id; }

  private:
    bool publish_event_json(const std::string& json) noexcept;
    bool publish_chunks(const JpegBuffer& jpeg, const std::string& snapshot_id) noexcept;

    EventDebouncer _debouncer;
    EventSerializer _serializer;
    std::string _device_id;
    RuntimeConfig::Mqtt _mqtt_cfg;
    ITransport* _transport;
    ILogger* _log;
};

// ---------------------------------------------------------------------------
// Template implementation
// ---------------------------------------------------------------------------

template <typename LabelFn>
PublishOutcome Publisher::publish_with_labels(const DetectionEvent& event, LabelFn&& resolver,
                                              std::optional<JpegBuffer> snapshot) noexcept {
    if (!_transport->connected()) {
        if (_log)
            _log->warn("Publisher: transport not connected, dropping event");
        return PublishOutcome::NotConnected;
    }

    // Debounce: check first detection's class_id (primary hit).
    if (!event.detections.empty()) {
        const auto& d = event.detections.front();
        const std::string label = resolver(d.class_id);
        if (!_debouncer.should_emit(d.class_id, label, event.event_ts_ms)) {
            return PublishOutcome::Suppressed;
        }
    }

    const std::string json =
        _serializer.serialize_with_labels(event, std::forward<LabelFn>(resolver));

    if (!publish_event_json(json)) {
        return PublishOutcome::TransportError;
    }

    if (snapshot.has_value() && !snapshot->data.empty()) {
        if (!publish_chunks(*snapshot, event.snapshot_id)) {
            return PublishOutcome::TransportError;
        }
    }

    return PublishOutcome::Ok;
}

} // namespace app
} // namespace wildlife