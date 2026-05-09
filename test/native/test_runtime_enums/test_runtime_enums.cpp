#include "wildlife/domain/runtime_enums.hpp"

#include <unity.h>

using namespace wildlife;

void setUp() {}
void tearDown() {}

// ---------------------------------------------------------------------------
// PowerMode
// ---------------------------------------------------------------------------

void test_power_mode_pir_deep_sleep() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::PirDeepSleep),
                          static_cast<int>(power_mode_from_string("pir_deep_sleep")));
}

void test_power_mode_always_on() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::AlwaysOn),
                          static_cast<int>(power_mode_from_string("always_on")));
}

void test_power_mode_unknown() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::Unknown),
                          static_cast<int>(power_mode_from_string("pir"))); // old value
    TEST_ASSERT_EQUAL_INT(static_cast<int>(PowerMode::Unknown),
                          static_cast<int>(power_mode_from_string("")));
}

void test_power_mode_round_trip() {
    for (auto m : {PowerMode::AlwaysOn, PowerMode::PirDeepSleep}) {
        TEST_ASSERT_EQUAL_INT(static_cast<int>(m),
                              static_cast<int>(power_mode_from_string(power_mode_to_string(m))));
    }
}

// ---------------------------------------------------------------------------
// SnapshotTransport
// ---------------------------------------------------------------------------

void test_snapshot_transport_mqtt_chunked() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotTransport::MqttChunked),
                          static_cast<int>(snapshot_transport_from_string("mqtt_chunked")));
}

void test_snapshot_transport_http_url() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotTransport::HttpUrl),
                          static_cast<int>(snapshot_transport_from_string("http_url")));
}

void test_snapshot_transport_old_value_unknown() {
    // "mqtt" (old config drift value) must be Unknown to surface drift at runtime
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotTransport::Unknown),
                          static_cast<int>(snapshot_transport_from_string("mqtt")));
}

void test_snapshot_transport_round_trip() {
    for (auto t : {SnapshotTransport::MqttChunked, SnapshotTransport::HttpUrl}) {
        TEST_ASSERT_EQUAL_INT(static_cast<int>(t), static_cast<int>(snapshot_transport_from_string(
                                                       snapshot_transport_to_string(t))));
    }
}

// ---------------------------------------------------------------------------
// SnapshotSource
// ---------------------------------------------------------------------------

void test_snapshot_source_sscma() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotSource::Sscma),
                          static_cast<int>(snapshot_source_from_string("sscma")));
}

void test_snapshot_source_esp32cam() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotSource::Esp32Cam),
                          static_cast<int>(snapshot_source_from_string("esp32cam")));
}

void test_snapshot_source_null() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotSource::Null),
                          static_cast<int>(snapshot_source_from_string("null")));
}

void test_snapshot_source_unknown() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(SnapshotSource::Unknown),
                          static_cast<int>(snapshot_source_from_string("camera")));
}

void test_snapshot_source_round_trip() {
    for (auto s : {SnapshotSource::Sscma, SnapshotSource::Esp32Cam, SnapshotSource::Null}) {
        TEST_ASSERT_EQUAL_INT(static_cast<int>(s), static_cast<int>(snapshot_source_from_string(
                                                       snapshot_source_to_string(s))));
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_power_mode_pir_deep_sleep);
    RUN_TEST(test_power_mode_always_on);
    RUN_TEST(test_power_mode_unknown);
    RUN_TEST(test_power_mode_round_trip);
    RUN_TEST(test_snapshot_transport_mqtt_chunked);
    RUN_TEST(test_snapshot_transport_http_url);
    RUN_TEST(test_snapshot_transport_old_value_unknown);
    RUN_TEST(test_snapshot_transport_round_trip);
    RUN_TEST(test_snapshot_source_sscma);
    RUN_TEST(test_snapshot_source_esp32cam);
    RUN_TEST(test_snapshot_source_null);
    RUN_TEST(test_snapshot_source_unknown);
    RUN_TEST(test_snapshot_source_round_trip);
    return UNITY_END();
}
