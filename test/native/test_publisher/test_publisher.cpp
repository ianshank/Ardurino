#include "../test_util/fake_clock.hpp"
#include "../test_util/fake_transport.hpp"
#include "../test_util/recording_logger.hpp"
#include "wildlife/app/event_debouncer.hpp"
#include "wildlife/app/event_serializer.hpp"
#include "wildlife/app/publisher.hpp"
#include "wildlife/domain/detection.hpp"
#include "wildlife/domain/runtime_config.hpp"

#include <unity.h>

using namespace wildlife;
using namespace wildlife::app;
using namespace wildlife::test;

static RuntimeConfig::Mqtt make_mqtt_cfg() {
    RuntimeConfig::Mqtt cfg;
    cfg.base_topic = "wildlife";
    cfg.qos = 0;
    cfg.retain = false;
    cfg.max_payload_kb = 4;
    return cfg;
}

static DetectionEvent make_event(uint64_t ts_ms) {
    DetectionEvent ev;
    Detection d;
    d.class_id = 1;
    d.score = 80;
    d.ts_ms = ts_ms;
    ev.detections.push_back(d);
    ev.event_ts_ms = ts_ms;
    ev.snapshot_id = "snap-001";
    return ev;
}

static Publisher make_publisher(FakeTransport& transport, RecordingLogger& logger) {
    DebouncerConfig dcfg;
    dcfg.debounce_ms = 100;
    return Publisher(EventDebouncer(dcfg, &logger), EventSerializer(), "test-device",
                     make_mqtt_cfg(), transport, &logger);
}

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------

void test_happy_path_publishes_event_topic() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    const auto ev = make_event(1000);
    const auto rc = pub.publish(ev, "bird", std::nullopt);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Ok), static_cast<int>(rc));
    TEST_ASSERT_EQUAL_UINT(1u, transport.publishes().size());
    TEST_ASSERT_TRUE(transport.publishes()[0].topic == "wildlife/test-device/event");
}

void test_not_connected_returns_not_connected() {
    FakeTransport transport(false); // disconnected
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    const auto rc = pub.publish(make_event(1000), "bird", std::nullopt);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::NotConnected), static_cast<int>(rc));
    TEST_ASSERT_EQUAL_UINT(0u, transport.publishes().size());
}

void test_debounce_suppresses_second_event_in_window() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    pub.publish(make_event(1000), "bird", std::nullopt);                 // first — emitted
    const auto rc = pub.publish(make_event(1050), "bird", std::nullopt); // within 100 ms window

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Suppressed), static_cast<int>(rc));
    TEST_ASSERT_EQUAL_UINT(1u, transport.publishes().size()); // only one published
}

void test_debounce_allows_event_after_window() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    pub.publish(make_event(1000), "bird", std::nullopt);
    const auto rc = pub.publish(make_event(1200), "bird", std::nullopt); // 200 ms later

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Ok), static_cast<int>(rc));
    TEST_ASSERT_EQUAL_UINT(2u, transport.publishes().size());
}

void test_snapshot_is_published_when_provided() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    JpegBuffer jpeg;
    jpeg.data = std::vector<uint8_t>(512, 0xAA);

    const auto rc = pub.publish(make_event(1000), "deer", std::make_optional(jpeg));

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Ok), static_cast<int>(rc));
    // At least 2 publishes: 1 event + ≥1 snapshot chunk
    TEST_ASSERT_TRUE(transport.publishes().size() >= 2u);

    bool has_snapshot_topic = false;
    for (const auto& p : transport.publishes()) {
        if (p.topic == "wildlife/test-device/snapshot")
            has_snapshot_topic = true;
    }
    TEST_ASSERT_TRUE(has_snapshot_topic);
}

void test_empty_snapshot_not_published() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    JpegBuffer empty_jpeg; // data is empty

    pub.publish(make_event(1000), "cat", std::make_optional(empty_jpeg));

    // Only the event topic, no snapshot topic
    for (const auto& p : transport.publishes()) {
        TEST_ASSERT_FALSE(p.topic == "wildlife/test-device/snapshot");
    }
}

void test_publish_transport_failure_returns_transport_error() {
    FakeTransport transport;
    transport.set_publish_ok(false);
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    const auto rc = pub.publish(make_event(1000), "bear", std::nullopt);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::TransportError), static_cast<int>(rc));
}

void test_no_detections_event_still_publishes() {
    FakeTransport transport;
    RecordingLogger logger;
    DebouncerConfig dcfg;
    dcfg.debounce_ms = 100;
    Publisher pub(EventDebouncer(dcfg, nullptr), EventSerializer(), "test-device", make_mqtt_cfg(),
                  transport, &logger);

    DetectionEvent ev;
    ev.event_ts_ms = 5000;
    const auto rc = pub.publish(ev, "", std::nullopt);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Ok), static_cast<int>(rc));
    TEST_ASSERT_TRUE(transport.publishes()[0].topic == "wildlife/test-device/event");
}

void test_publish_with_labels_template() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);

    const auto ev = make_event(2000);
    const auto rc = pub.publish_with_labels(
        ev, [](int32_t id) -> std::string { return id == 1 ? "cat" : ""; }, std::nullopt);

    TEST_ASSERT_EQUAL_INT(static_cast<int>(PublishOutcome::Ok), static_cast<int>(rc));
    // Payload should contain "cat"
    const auto& payload = transport.publishes().back().payload;
    const std::string json(payload.begin(), payload.end());
    TEST_ASSERT_TRUE(json.find("cat") != std::string::npos);
}

void test_device_id_accessor_returns_id() {
    FakeTransport transport;
    RecordingLogger logger;
    auto pub = make_publisher(transport, logger);
    TEST_ASSERT_EQUAL_STRING("test-device", pub.device_id().c_str());
}

void test_empty_device_id_logs_warning() {
    FakeTransport transport;
    RecordingLogger logger;
    DebouncerConfig dcfg;
    dcfg.debounce_ms = 100;
    Publisher pub(EventDebouncer(dcfg, nullptr), EventSerializer(),
                  "", // empty device_id
                  make_mqtt_cfg(), transport, &logger);

    // Construction with empty device_id must have logged a warning.
    TEST_ASSERT_TRUE(logger.has_level(wildlife::LogLevel::Warn));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_happy_path_publishes_event_topic);
    RUN_TEST(test_not_connected_returns_not_connected);
    RUN_TEST(test_debounce_suppresses_second_event_in_window);
    RUN_TEST(test_debounce_allows_event_after_window);
    RUN_TEST(test_snapshot_is_published_when_provided);
    RUN_TEST(test_empty_snapshot_not_published);
    RUN_TEST(test_publish_transport_failure_returns_transport_error);
    RUN_TEST(test_no_detections_event_still_publishes);
    RUN_TEST(test_publish_with_labels_template);
    RUN_TEST(test_device_id_accessor_returns_id);
    RUN_TEST(test_empty_device_id_logs_warning);
    return UNITY_END();
}