#include "wildlife/config/config_subsystem.hpp"

#include "wildlife/domain/runtime_enums.hpp"
#include <algorithm>
#include <cstdio>
#include <string>

namespace wildlife {
namespace config {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {

using J = json::JType;

static std::string get_str(const JVal& obj, const char* key,
                            std::string_view def) noexcept {
    const auto& v = obj[key];
    return v.type() == J::Str ? std::string(v.as_str()) : std::string(def);
}

static bool get_bool(const JVal& obj, const char* key, bool def) noexcept {
    const auto& v = obj[key];
    return v.type() == J::Bool ? v.as_bool() : def;
}

static int64_t get_int(const JVal& obj, const char* key, int64_t def) noexcept {
    const auto& v = obj[key];
    return (v.type() == J::Int64 || v.type() == J::Double) ? v.as_int() : def;
}

static double get_double(const JVal& obj, const char* key, double def) noexcept {
    const auto& v = obj[key];
    return (v.type() == J::Double || v.type() == J::Int64) ? v.as_double() : def;
}

} // namespace

// ---------------------------------------------------------------------------
// ConfigParser — section parsers
// ---------------------------------------------------------------------------

void ConfigParser::parse_wifi(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["wifi"];
    if (s.is_null()) return;
    out.wifi.ap_name      = get_str (s, "ap_name",      out.wifi.ap_name);
    out.wifi.ap_password  = get_str (s, "ap_password",  out.wifi.ap_password);
    out.wifi.sta_timeout_s= static_cast<uint16_t>(get_int(s, "sta_timeout_s",
                                                           out.wifi.sta_timeout_s));
}

void ConfigParser::parse_mqtt(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["mqtt"];
    if (s.is_null()) return;
    out.mqtt.host           = get_str (s, "host",           out.mqtt.host);
    out.mqtt.port           = static_cast<uint16_t>(get_int(s, "port",   out.mqtt.port));
    out.mqtt.tls            = get_bool(s, "tls",            out.mqtt.tls);
    out.mqtt.username       = get_str (s, "username",       out.mqtt.username);
    out.mqtt.password       = get_str (s, "password",       out.mqtt.password);
    out.mqtt.client_id      = get_str (s, "client_id",      out.mqtt.client_id);
    out.mqtt.base_topic     = get_str (s, "base_topic",     out.mqtt.base_topic);
    out.mqtt.qos            = static_cast<uint8_t> (get_int(s, "qos",   out.mqtt.qos));
    out.mqtt.retain         = get_bool(s, "retain",         out.mqtt.retain);
    out.mqtt.max_payload_kb = static_cast<uint16_t>(get_int(s, "max_payload_kb",
                                                            out.mqtt.max_payload_kb));
    out.mqtt.keepalive_s    = static_cast<uint16_t>(get_int(s, "keepalive_s",
                                                            out.mqtt.keepalive_s));
}

void ConfigParser::parse_ota(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["ota"];
    if (s.is_null()) return;
    out.ota.enabled          = get_bool(s, "enabled",          out.ota.enabled);
    out.ota.channel          = get_str (s, "channel",          out.ota.channel);
    out.ota.url              = get_str (s, "url",              out.ota.url);
    out.ota.check_interval_h = static_cast<uint8_t>(get_int(s, "check_interval_h",
                                                             out.ota.check_interval_h));
}

void ConfigParser::parse_inference(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["inference"];
    if (s.is_null()) return;
    out.inference.min_score         = static_cast<uint8_t>(get_int(s, "min_score",
                                                           out.inference.min_score));
    out.inference.debounce_ms       = static_cast<uint32_t>(get_int(s, "debounce_ms",
                                                            out.inference.debounce_ms));
    out.inference.poll_interval_ms  = static_cast<uint32_t>(get_int(s, "poll_interval_ms",
                                                            out.inference.poll_interval_ms));
    out.inference.prewake_warmup_ms = static_cast<uint32_t>(get_int(s, "prewake_warmup_ms",
                                                            out.inference.prewake_warmup_ms));
    out.inference.sscma_invoke_times = static_cast<uint8_t>(get_int(s, "sscma_invoke_times",
                                                            out.inference.sscma_invoke_times));
    out.inference.sscma_filter      = static_cast<uint8_t>(get_int(s, "sscma_filter",
                                                            out.inference.sscma_filter));
    out.inference.sscma_encode      = static_cast<uint8_t>(get_int(s, "sscma_encode",
                                                            out.inference.sscma_encode));
    const auto& arr = s["classes_of_interest"];
    if (arr.type() == json::JType::Array) {
        out.inference.classes_of_interest.clear();
        for (std::size_t i = 0; i < arr.size(); ++i) {
            if (arr[i].type() == json::JType::Str) {
                out.inference.classes_of_interest.emplace_back(arr[i].as_str());
            }
        }
    }
}

void ConfigParser::parse_snapshot(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["snapshot"];
    if (s.is_null()) return;
    out.snapshot.enabled      = get_bool(s, "enabled",      out.snapshot.enabled);
    out.snapshot.transport    = get_str (s, "transport",    out.snapshot.transport);
    out.snapshot.source       = get_str (s, "source",       out.snapshot.source);
    out.snapshot.jpeg_quality = static_cast<uint8_t> (get_int(s, "jpeg_quality",
                                                              out.snapshot.jpeg_quality));
    out.snapshot.max_kb       = static_cast<uint16_t>(get_int(s, "max_kb",
                                                              out.snapshot.max_kb));
    out.snapshot.http_url     = get_str(s, "http_url",      out.snapshot.http_url);
}

void ConfigParser::parse_power(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["power"];
    if (s.is_null()) return;
    out.power.mode           = get_str(s, "mode",           out.power.mode);
    out.power.mode_enum      = power_mode_from_string(out.power.mode);
    out.power.wake_level     = static_cast<uint8_t> (get_int(s, "wake_level",
                                                             out.power.wake_level));
    out.power.sleep_seconds  = static_cast<uint32_t>(get_int(s, "sleep_seconds",
                                                             out.power.sleep_seconds));
    out.power.active_seconds = static_cast<uint32_t>(get_int(s, "active_seconds",
                                                             out.power.active_seconds));
    out.power.low_warn_mv    = static_cast<int32_t> (get_int(s, "low_warn_mv",
                                                             out.power.low_warn_mv));
    out.power.low_cutoff_mv  = static_cast<int32_t> (get_int(s, "low_cutoff_mv",
                                                             out.power.low_cutoff_mv));
}

void ConfigParser::parse_battery(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["battery"];
    if (s.is_null()) return;
    out.battery.divider_ratio = get_double(s, "divider_ratio", out.battery.divider_ratio);
    out.battery.vref_mv       = static_cast<int32_t>(get_int(s, "vref_mv",
                                                             out.battery.vref_mv));
    out.battery.samples       = static_cast<uint8_t>(get_int(s, "samples",
                                                             out.battery.samples));
}

void ConfigParser::parse_labels(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["labels"];
    if (s.is_null()) return;
    out.labels.path          = get_str(s, "path",          out.labels.path);
    out.labels.expected_hash = get_str(s, "expected_hash", out.labels.expected_hash);
}

void ConfigParser::parse_logging(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["logging"];
    if (s.is_null()) return;
    out.logging.level             = get_str(s, "level",             out.logging.level);
    out.logging.mqtt_topic_suffix = get_str(s, "mqtt_topic_suffix", out.logging.mqtt_topic_suffix);
}

bool ConfigParser::parse(const JVal& doc, RuntimeConfig& out,
                          std::string& error_msg) noexcept {
    if (doc.type() != json::JType::Object) {
        error_msg = "root must be a JSON object";
        if (_log) _log->error(error_msg);
        return false;
    }
    const auto& sv = doc["schema_version"];
    out.schema_version = static_cast<int>(get_int(doc, "schema_version",
                                                  RuntimeConfig::kCurrentSchemaVersion));
    out.device_id = get_str(doc, "device_id", out.device_id);

    parse_wifi     (doc, out);
    parse_mqtt     (doc, out);
    parse_ota      (doc, out);
    parse_inference(doc, out);
    parse_snapshot (doc, out);
    parse_power    (doc, out);
    parse_battery  (doc, out);
    parse_labels   (doc, out);
    parse_logging  (doc, out);
    parse_network  (doc, out);
    (void)sv;

    if (_log) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "Config parsed: schema_version=%d device_id=%s",
                      out.schema_version, out.device_id.c_str());
        _log->info(buf);
    }
    return true;
}

// ---------------------------------------------------------------------------
// DefaultsMerger
// ---------------------------------------------------------------------------

JVal DefaultsMerger::merge(const JVal& defaults, const JVal& user) const noexcept {
    using J = json::JType;
    // Non-object types: user wins unless user is null (treat null as absent).
    if (defaults.type() != J::Object || user.type() != J::Object) {
        return user.is_null() ? defaults : user;
    }
    // Both are objects: start with a copy of defaults, then apply user values.
    JVal result = defaults;  // deep copy
    for (const auto& [key, user_val] : user.as_object_ref()) {
        const JVal& def_val = defaults[key];
        if (def_val.type() == J::Object && user_val.type() == J::Object) {
            result.set(key, merge(def_val, user_val));
        } else {
            result.set(key, user_val);  // user wins (incl. unknown new keys)
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// SchemaMigrator
// ---------------------------------------------------------------------------

void SchemaMigrator::register_migration(int from_version, MigrateFn fn) {
    _migrations.push_back({from_version, std::move(fn)});
}

bool SchemaMigrator::migrate(JVal& doc, int target_version) noexcept {
    int ver = static_cast<int>(doc["schema_version"].as_int(0));

    if (ver > target_version) return false;  // future schema — cannot downgrade
    if (ver == target_version) return true;  // already current

    // Walk the migration chain: ver → ver+1 → … → target_version
    for (int v = ver; v < target_version; ++v) {
        bool found = false;
        for (auto& e : _migrations) {
            if (e.from == v) {
                if (!e.fn(doc)) return false;
                doc.set("schema_version", JVal{static_cast<int64_t>(v + 1)});
                found = true;
                break;
            }
        }
        if (!found) return false;  // no migration registered for this step
    }
    return true;
}

void ConfigParser::parse_network(const JVal& root, RuntimeConfig& out) noexcept {
    const auto& s = root["network"];
    if (s.is_null()) return;
    out.network.retry.base_ms      = static_cast<uint32_t>(get_int(s, "retry_base_ms",
                                                            out.network.retry.base_ms));
    out.network.retry.max_ms       = static_cast<uint32_t>(get_int(s, "retry_max_ms",
                                                            out.network.retry.max_ms));
    out.network.retry.max_attempts = static_cast<uint8_t>(get_int(s, "retry_max_attempts",
                                                            out.network.retry.max_attempts));
    out.network.retry.jitter_pct   = static_cast<uint8_t>(get_int(s, "retry_jitter_pct",
                                                            out.network.retry.jitter_pct));
}

} // namespace config
} // namespace wildlife
