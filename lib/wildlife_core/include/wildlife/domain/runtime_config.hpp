#pragma once

// Wildlife Core — Mutable runtime configuration.
// Loaded from LittleFS (/config.json), provisioned via captive portal.
// All fields have safe defaults so a missing key never crashes boot.

#include <cstdint>
#include <string>
#include <vector>

#include "wildlife/app/retry_policy.hpp"
#include "wildlife/domain/runtime_enums.hpp"

namespace wildlife {

struct RuntimeConfig {
    static constexpr int kCurrentSchemaVersion = 1;

    int         schema_version{kCurrentSchemaVersion};
    std::string device_id;   // generated on first boot if empty

    // -----------------------------------------------------------------------
    struct Wifi {
        std::string ap_name     {"wildlife-setup"};
        std::string ap_password;
        uint16_t    sta_timeout_s{30};
    } wifi;

    // -----------------------------------------------------------------------
    struct Mqtt {
        std::string host;
        uint16_t    port          {1883};
        bool        tls           {false};
        std::string username;
        std::string password;
        std::string client_id;
        std::string base_topic    {"wildlife"};
        uint8_t     qos           {1};
        bool        retain        {false};
        uint16_t    max_payload_kb{32};
        uint16_t    keepalive_s   {60};
    } mqtt;

    // -----------------------------------------------------------------------
    struct Ota {
        bool        enabled          {false};
        std::string channel          {"release"};
        std::string url;
        uint8_t     check_interval_h {24};
    } ota;

    // -----------------------------------------------------------------------
    struct Inference {
        uint8_t                  min_score        {60};
        uint32_t                 debounce_ms      {5000};
        std::vector<std::string> classes_of_interest; // empty = all
        uint32_t                 poll_interval_ms {500};
        uint32_t                 prewake_warmup_ms{1500};
        uint8_t                  sscma_invoke_times{1};
        uint8_t                  sscma_filter      {0};
        uint8_t                  sscma_encode      {1};
    } inference;

    // -----------------------------------------------------------------------
    struct Snapshot {
        bool        enabled      {true};
        std::string transport    {"mqtt_chunked"}; // "mqtt_chunked" | "http_url"
        std::string source       {"sscma"};        // "sscma" | "esp32cam"
        uint8_t     jpeg_quality {85};
        uint16_t    max_kb       {30};
        std::string http_url;
    } snapshot;

    // -----------------------------------------------------------------------
    struct Network {
        wildlife::app::RetryConfig retry; // exponential back-off for reconnect
    } network;

    // -----------------------------------------------------------------------
    struct Power {
        std::string mode          {"pir_deep_sleep"}; // "always_on" | "pir_deep_sleep"
        PowerMode   mode_enum     {PowerMode::PirDeepSleep}; // derived from mode by parser
        uint8_t     wake_level    {1};
        uint32_t    sleep_seconds {300};
        uint32_t    active_seconds{30};
        int32_t     low_warn_mv   {3500};
        int32_t     low_cutoff_mv {3200};
    } power;

    // -----------------------------------------------------------------------
    struct Battery {
        double  divider_ratio{2.0};
        int32_t vref_mv      {3300};
        uint8_t samples      {8};
    } battery;

    // -----------------------------------------------------------------------
    struct Labels {
        std::string path          {"/labels.json"};
        std::string expected_hash;  // empty = skip hash check
    } labels;

    // -----------------------------------------------------------------------
    struct Logging {
        std::string level             {"info"};
        std::string mqtt_topic_suffix {"log"};
    } logging;
};

} // namespace wildlife
