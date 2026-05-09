#include "wildlife/app/publisher.hpp"

#include "wildlife/core/units.hpp"

#include <cstdio>

namespace wildlife {
namespace app {

Publisher::Publisher(EventDebouncer debouncer, EventSerializer serializer, std::string device_id,
                     const RuntimeConfig::Mqtt& mqtt_cfg, ITransport& transport,
                     ILogger* log) noexcept
    : _debouncer(std::move(debouncer)), _serializer(std::move(serializer)),
      _device_id(std::move(device_id)), _mqtt_cfg(mqtt_cfg), _transport(&transport), _log(log) {
    if (_device_id.empty() && _log) {
        _log->warn("Publisher: device_id is empty — MQTT topics will be malformed");
    }
}

PublishOutcome Publisher::publish(const DetectionEvent& event, const std::string& resolved_label,
                                  std::optional<JpegBuffer> snapshot) noexcept {
    if (!_transport->connected()) {
        if (_log)
            _log->warn("Publisher: transport not connected, dropping event");
        return PublishOutcome::NotConnected;
    }

    // Debounce on first detection using the pre-resolved label.
    if (!event.detections.empty()) {
        const auto& d = event.detections.front();
        if (!_debouncer.should_emit(d.class_id, resolved_label, event.event_ts_ms)) {
            return PublishOutcome::Suppressed;
        }
    }

    // Serialise without a label resolver — callers already resolved externally.
    const std::string json = _serializer.serialize(event);
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

bool Publisher::publish_event_json(const std::string& json) noexcept {
    const std::string topic = _mqtt_cfg.base_topic + "/" + _device_id + "/event";
    const auto* payload = reinterpret_cast<const uint8_t*>(json.data());
    const bool ok =
        _transport->publish(topic, payload, json.size(), _mqtt_cfg.qos, _mqtt_cfg.retain);
    if (!ok && _log) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Publisher: event publish failed (topic=%s, len=%zu)",
                      topic.c_str(), json.size());
        _log->warn(buf);
    }
    return ok;
}

bool Publisher::publish_chunks(const JpegBuffer& jpeg, const std::string& snapshot_id) noexcept {
    const std::size_t chunk_bytes =
        static_cast<std::size_t>(_mqtt_cfg.max_payload_kb) * wildlife::kBytesPerKb;

    const auto chunks = chunk_jpeg(jpeg, chunk_bytes, snapshot_id);
    if (chunks.empty()) {
        if (_log)
            _log->warn("Publisher: chunk_jpeg returned empty — skipping snapshot");
        return false;
    }

    // Topic layout encodes correlation/reassembly metadata so subscribers can
    // group multi-chunk JPEGs without parsing the binary payload:
    //   <base>/<device>/snapshot/<snapshot_id>/manifest        (JSON: total, sha256_hex)
    //   <base>/<device>/snapshot/<snapshot_id>/<seq>/<total>   (binary chunk)
    const std::string base_topic =
        _mqtt_cfg.base_topic + "/" + _device_id + "/snapshot/" + snapshot_id;

    // Publish manifest first so consumers can pre-allocate / dedupe before
    // chunks start streaming. seq==0 carries the SHA-256 of the full JPEG.
    {
        const std::string manifest_topic = base_topic + "/manifest";
        char manifest[160];
        const int n = std::snprintf(
            manifest, sizeof(manifest), "{\"snapshot_id\":\"%s\",\"total\":%u,\"sha256\":\"%s\"}",
            snapshot_id.c_str(), chunks.front().total, chunks.front().sha256_hex.c_str());
        if (n > 0) {
            const auto* payload = reinterpret_cast<const uint8_t*>(manifest);
            const bool ok =
                _transport->publish(manifest_topic, payload, static_cast<std::size_t>(n),
                                    _mqtt_cfg.qos, _mqtt_cfg.retain);
            if (!ok) {
                if (_log) {
                    char buf[160];
                    std::snprintf(buf, sizeof(buf),
                                  "Publisher: snapshot manifest publish failed (topic=%s)",
                                  manifest_topic.c_str());
                    _log->warn(buf);
                }
                return false;
            }
        }
    }

    bool all_ok = true;
    for (const auto& chunk : chunks) {
        char suffix[32];
        std::snprintf(suffix, sizeof(suffix), "/%u/%u", chunk.seq, chunk.total);
        const std::string topic = base_topic + suffix;
        const bool ok = _transport->publish(topic, chunk.data.data(), chunk.data.size(),
                                            _mqtt_cfg.qos, _mqtt_cfg.retain);
        if (!ok) {
            if (_log) {
                char buf[80];
                std::snprintf(buf, sizeof(buf), "Publisher: snapshot chunk %u/%u publish failed",
                              chunk.seq + 1u, chunk.total);
                _log->warn(buf);
            }
            all_ok = false;
            break; // stop on first failure; incomplete stream is useless
        }
    }

    if (_log && all_ok) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "Publisher: snapshot %s sent (%zu chunks, %zu bytes)",
                      snapshot_id.c_str(), chunks.size(), jpeg.data.size());
        _log->debug(buf);
    }
    return all_ok;
}

} // namespace app
} // namespace wildlife