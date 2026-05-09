#include "../test_util/null_logger.hpp"
#include "wildlife/config/config_subsystem.hpp"
#include "wildlife/domain/runtime_enums.hpp"
#include "wildlife/json/mini_json.hpp"

#include <unity.h>

using namespace wildlife;
using namespace wildlife::config;
using namespace wildlife::json;
using wildlife::test::NullLogger;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static JVal default_doc() {
    auto v = parse(R"({
        "schema_version": 1,
        "device_id": "test-device",
        "wifi":      {"ap_name":"TestAP","ap_password":"","sta_timeout_s":20},
        "mqtt":      {"host":"mqtt.local","port":1883,"tls":false,
                      "username":"","password":"","client_id":"cli1",
                      "base_topic":"wl/events","qos":1,"retain":false,
                      "max_payload_kb":64,"keepalive_s":60},
        "ota":       {"enabled":true,"channel":"stable","url":"","check_interval_h":24},
        "inference": {"min_score":60,"debounce_ms":5000,
                      "classes_of_interest":[],"poll_interval_ms":200,
                      "prewake_warmup_ms":500},
        "snapshot":  {"enabled":true,"transport":"mqtt","source":"sscma",
                      "jpeg_quality":75,"max_kb":60,"http_url":""},
        "power":     {"mode":"pir","wake_level":1,"sleep_seconds":60,
                      "active_seconds":30,"low_warn_mv":3400,"low_cutoff_mv":3200},
        "battery":   {"divider_ratio":2.0,"vref_mv":3300,"samples":8},
        "labels":    {"path":"/labels.json","expected_hash":""},
        "logging":   {"level":"info","mqtt_topic_suffix":"/log"}
    })");
    TEST_ASSERT_TRUE(v.has_value());
    return std::move(*v);
}

// ---------------------------------------------------------------------------
// ConfigParser tests
// ---------------------------------------------------------------------------

void test_config_parses_device_id() {
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_TRUE(p.parse(default_doc(), cfg, err));
    TEST_ASSERT_EQUAL_STRING("test-device", cfg.device_id.c_str());
}

void test_config_parses_mqtt_host() {
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_TRUE(p.parse(default_doc(), cfg, err));
    TEST_ASSERT_EQUAL_STRING("mqtt.local", cfg.mqtt.host.c_str());
    TEST_ASSERT_EQUAL_UINT(1883u, cfg.mqtt.port);
}

void test_config_parses_inference_min_score() {
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_TRUE(p.parse(default_doc(), cfg, err));
    TEST_ASSERT_EQUAL_UINT(60u, cfg.inference.min_score);
}

void test_config_parses_classes_of_interest_empty() {
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_TRUE(p.parse(default_doc(), cfg, err));
    TEST_ASSERT_EQUAL_UINT(0u, cfg.inference.classes_of_interest.size());
}

void test_config_parses_classes_of_interest_values() {
    auto doc = parse(R"({"schema_version":1,"inference":{"classes_of_interest":["dog","cat"]}})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    p.parse(*doc, cfg, err);
    TEST_ASSERT_EQUAL_UINT(2u, cfg.inference.classes_of_interest.size());
    TEST_ASSERT_EQUAL_STRING("dog", cfg.inference.classes_of_interest[0].c_str());
}

void test_config_fails_on_non_object() {
    auto doc = parse("[1,2,3]");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_FALSE(p.parse(*doc, cfg, err));
    TEST_ASSERT_FALSE(err.empty());
}

void test_config_missing_section_uses_defaults() {
    auto doc = parse(R"({"schema_version":1,"device_id":"abc"})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig defaults;
    RuntimeConfig cfg = defaults; // copy defaults
    std::string err;
    TEST_ASSERT_TRUE(p.parse(*doc, cfg, err));
    // mqtt.port should still equal default (1883)
    TEST_ASSERT_EQUAL_UINT(defaults.mqtt.port, cfg.mqtt.port);
}

// ---------------------------------------------------------------------------
// DefaultsMerger tests
// ---------------------------------------------------------------------------

void test_merger_user_wins_scalar() {
    auto d = parse(R"({"a":1,"b":2})");
    auto u = parse(R"({"a":99})");
    TEST_ASSERT_TRUE(d.has_value());
    TEST_ASSERT_TRUE(u.has_value());
    DefaultsMerger m;
    JVal result = m.merge(*d, *u);
    TEST_ASSERT_EQUAL_INT64(99, result["a"].as_int());
    TEST_ASSERT_EQUAL_INT64(2, result["b"].as_int());
}

void test_merger_deep_merge_nested() {
    auto d = parse(R"({"outer":{"x":1,"y":2}})");
    auto u = parse(R"({"outer":{"x":99}})");
    TEST_ASSERT_TRUE(d.has_value());
    TEST_ASSERT_TRUE(u.has_value());
    DefaultsMerger m;
    JVal result = m.merge(*d, *u);
    TEST_ASSERT_EQUAL_INT64(99, result["outer"]["x"].as_int());
    TEST_ASSERT_EQUAL_INT64(2, result["outer"]["y"].as_int());
}

void test_merger_unknown_user_keys_preserved() {
    auto d = parse(R"({"a":1})");
    auto u = parse(R"({"b":2})");
    TEST_ASSERT_TRUE(d.has_value());
    TEST_ASSERT_TRUE(u.has_value());
    DefaultsMerger m;
    JVal result = m.merge(*d, *u);
    TEST_ASSERT_EQUAL_INT64(1, result["a"].as_int());
    TEST_ASSERT_EQUAL_INT64(2, result["b"].as_int());
}

void test_merger_null_user_falls_back_to_default() {
    JVal d{static_cast<int64_t>(42)};
    JVal u{}; // null
    DefaultsMerger m;
    JVal result = m.merge(d, u);
    TEST_ASSERT_EQUAL_INT64(42, result.as_int());
}

// ---------------------------------------------------------------------------
// SchemaMigrator tests
// ---------------------------------------------------------------------------

void test_migrator_already_current() {
    auto doc = parse(R"({"schema_version":1})");
    TEST_ASSERT_TRUE(doc.has_value());
    SchemaMigrator sm;
    TEST_ASSERT_TRUE(sm.migrate(*doc, 1));
    TEST_ASSERT_EQUAL_INT64(1, (*doc)["schema_version"].as_int());
}

void test_migrator_v0_to_v1() {
    auto doc = parse(R"({"schema_version":0,"old_key":"x"})");
    TEST_ASSERT_TRUE(doc.has_value());
    SchemaMigrator sm;
    sm.register_migration(0, [](JVal& d) {
        // v0→v1: rename old_key → new_key
        auto v = d["old_key"];
        d.set("new_key", v);
        d.remove("old_key");
        return true;
    });
    TEST_ASSERT_TRUE(sm.migrate(*doc, 1));
    TEST_ASSERT_EQUAL_INT64(1, (*doc)["schema_version"].as_int());
    TEST_ASSERT_EQUAL_STRING("x", std::string((*doc)["new_key"].as_str()).c_str());
}

void test_migrator_future_version_fails() {
    auto doc = parse(R"({"schema_version":99})");
    TEST_ASSERT_TRUE(doc.has_value());
    SchemaMigrator sm;
    TEST_ASSERT_FALSE(sm.migrate(*doc, 1));
}

void test_migrator_missing_step_fails() {
    // version 0 with no migration registered
    auto doc = parse(R"({"schema_version":0})");
    TEST_ASSERT_TRUE(doc.has_value());
    SchemaMigrator sm;
    TEST_ASSERT_FALSE(sm.migrate(*doc, 1));
}

void test_migrator_fn_returning_false_fails() {
    auto doc = parse(R"({"schema_version":0})");
    TEST_ASSERT_TRUE(doc.has_value());
    SchemaMigrator sm;
    sm.register_migration(0, [](JVal&) { return false; });
    TEST_ASSERT_FALSE(sm.migrate(*doc, 1));
}

void test_config_parser_with_logger_on_error() {
    // Exercises ConfigParser with a non-null logger on the error path.
    NullLogger log;
    ConfigParser p{&log};
    auto doc = parse("[1,2,3]"); // array → not an object
    TEST_ASSERT_TRUE(doc.has_value());
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_FALSE(p.parse(*doc, cfg, err));
    TEST_ASSERT_FALSE(err.empty());
}

void test_config_parser_with_logger_on_success() {
    // Exercises ConfigParser with a non-null logger on the success path.
    NullLogger log;
    ConfigParser p{&log};
    auto doc = parse(R"({"schema_version":1,"device_id":"logged-dev"})");
    TEST_ASSERT_TRUE(doc.has_value());
    RuntimeConfig cfg;
    std::string err;
    TEST_ASSERT_TRUE(p.parse(*doc, cfg, err));
    TEST_ASSERT_EQUAL_STRING("logged-dev", cfg.device_id.c_str());
}

// ---------------------------------------------------------------------------
// S1 new config tests
// ---------------------------------------------------------------------------

void test_config_parses_inference_sscma_fields() {
    auto doc = parse(R"({"schema_version":1,"inference":{
        "sscma_invoke_times":3,"sscma_filter":1,"sscma_encode":0}})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    p.parse(*doc, cfg, err);
    TEST_ASSERT_EQUAL_UINT(3u, cfg.inference.sscma_invoke_times);
    TEST_ASSERT_EQUAL_UINT(1u, cfg.inference.sscma_filter);
    TEST_ASSERT_EQUAL_UINT(0u, cfg.inference.sscma_encode);
}

void test_config_sscma_defaults_when_section_missing() {
    auto doc = parse(R"({"schema_version":1})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig defaults;
    RuntimeConfig cfg = defaults;
    std::string err;
    p.parse(*doc, cfg, err);
    TEST_ASSERT_EQUAL_UINT(defaults.inference.sscma_invoke_times, cfg.inference.sscma_invoke_times);
    TEST_ASSERT_EQUAL_UINT(defaults.inference.sscma_filter, cfg.inference.sscma_filter);
    TEST_ASSERT_EQUAL_UINT(defaults.inference.sscma_encode, cfg.inference.sscma_encode);
}

void test_config_parses_network_retry() {
    auto doc = parse(R"({"schema_version":1,"network":{
        "retry_base_ms":2000,"retry_max_ms":60000,
        "retry_max_attempts":5,"retry_jitter_pct":10}})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig cfg;
    std::string err;
    p.parse(*doc, cfg, err);
    TEST_ASSERT_EQUAL_UINT32(2000u, cfg.network.retry.base_ms);
    TEST_ASSERT_EQUAL_UINT32(60000u, cfg.network.retry.max_ms);
    TEST_ASSERT_EQUAL_UINT(5u, cfg.network.retry.max_attempts);
    TEST_ASSERT_EQUAL_UINT(10u, cfg.network.retry.jitter_pct);
}

void test_config_network_defaults_when_section_missing() {
    auto doc = parse(R"({"schema_version":1})");
    TEST_ASSERT_TRUE(doc.has_value());
    ConfigParser p;
    RuntimeConfig defaults;
    RuntimeConfig cfg = defaults;
    std::string err;
    p.parse(*doc, cfg, err);
    TEST_ASSERT_EQUAL_UINT32(defaults.network.retry.base_ms, cfg.network.retry.base_ms);
    TEST_ASSERT_EQUAL_UINT32(defaults.network.retry.max_ms, cfg.network.retry.max_ms);
    TEST_ASSERT_EQUAL_UINT(defaults.network.retry.max_attempts, cfg.network.retry.max_attempts);
    TEST_ASSERT_EQUAL_UINT(defaults.network.retry.jitter_pct, cfg.network.retry.jitter_pct);
}

void test_config_parses_power_mode_enum() {
    auto doc_pir = parse(R"({"schema_version":1,"power":{"mode":"pir_deep_sleep"}})");
    auto doc_always = parse(R"({"schema_version":1,"power":{"mode":"always_on"}})");
    TEST_ASSERT_TRUE(doc_pir.has_value());
    TEST_ASSERT_TRUE(doc_always.has_value());

    ConfigParser p;
    RuntimeConfig cfg_pir, cfg_always;
    std::string err;
    p.parse(*doc_pir, cfg_pir, err);
    p.parse(*doc_always, cfg_always, err);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::PirDeepSleep),
                          static_cast<int>(cfg_pir.power.mode_enum));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::AlwaysOn),
                          static_cast<int>(cfg_always.power.mode_enum));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_config_parses_device_id);
    RUN_TEST(test_config_parses_mqtt_host);
    RUN_TEST(test_config_parses_inference_min_score);
    RUN_TEST(test_config_parses_classes_of_interest_empty);
    RUN_TEST(test_config_parses_classes_of_interest_values);
    RUN_TEST(test_config_fails_on_non_object);
    RUN_TEST(test_config_missing_section_uses_defaults);
    RUN_TEST(test_merger_user_wins_scalar);
    RUN_TEST(test_merger_deep_merge_nested);
    RUN_TEST(test_merger_unknown_user_keys_preserved);
    RUN_TEST(test_merger_null_user_falls_back_to_default);
    RUN_TEST(test_migrator_already_current);
    RUN_TEST(test_migrator_v0_to_v1);
    RUN_TEST(test_migrator_future_version_fails);
    RUN_TEST(test_migrator_missing_step_fails);
    RUN_TEST(test_migrator_fn_returning_false_fails);
    RUN_TEST(test_config_parser_with_logger_on_error);
    RUN_TEST(test_config_parser_with_logger_on_success);
    RUN_TEST(test_config_parses_inference_sscma_fields);
    RUN_TEST(test_config_sscma_defaults_when_section_missing);
    RUN_TEST(test_config_parses_network_retry);
    RUN_TEST(test_config_network_defaults_when_section_missing);
    RUN_TEST(test_config_parses_power_mode_enum);
    return UNITY_END();
}
