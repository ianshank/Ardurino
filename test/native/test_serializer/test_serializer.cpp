#include "wildlife/app/event_serializer.hpp"
#include "wildlife/json/mini_json.hpp"

#include <unity.h>

using namespace wildlife;
using namespace wildlife::app;

void setUp() {}
void tearDown() {}

static DetectionEvent make_event() {
    DetectionEvent ev;
    ev.event_ts_ms = 123456789ULL;
    ev.snapshot_id = "snap-001";
    ev.meta.device_id = "dev-1";
    ev.meta.fw_version = "1.0.0";
    ev.meta.board_id = "xiao";
    ev.meta.rssi = -72;
    ev.meta.battery_mv = 3800;

    Detection d;
    d.class_id = 2;
    d.score = 85;
    d.bbox = BBox{10, 20, 30, 40};
    d.ts_ms = 123456700ULL;
    ev.detections.push_back(d);
    return ev;
}

void test_serializer_produces_valid_json() {
    EventSerializer ser;
    auto json_str = ser.serialize(make_event());
    auto v = json::parse(json_str);
    TEST_ASSERT_TRUE(v.has_value());
}

void test_serializer_schema_field() {
    EventSerializer ser;
    auto json_str = ser.serialize(make_event());
    auto v = json::parse(json_str);
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("detection.v1", std::string((*v)["schema"].as_str()).c_str());
}

void test_serializer_event_ts() {
    EventSerializer ser;
    auto v = json::parse(ser.serialize(make_event()));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_UINT64(123456789ULL, static_cast<uint64_t>((*v)["event_ts_ms"].as_int()));
}

void test_serializer_one_detection() {
    EventSerializer ser;
    auto v = json::parse(ser.serialize(make_event()));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_UINT(1u, (*v)["detections"].size());
    TEST_ASSERT_EQUAL_INT64(2, (*v)["detections"][0]["class_id"].as_int());
    TEST_ASSERT_EQUAL_INT64(85, (*v)["detections"][0]["score"].as_int());
}

void test_serializer_with_labels() {
    EventSerializer ser;
    auto json_str = ser.serialize_with_labels(make_event(), [](int32_t id) -> std::string {
        if (id == 2)
            return "dog";
        return "unknown";
    });
    auto v = json::parse(json_str);
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("dog", std::string((*v)["detections"][0]["label"].as_str()).c_str());
}

void test_json_escape_quotes() {
    std::string s = EventSerializer::json_escape("say \"hi\"");
    TEST_ASSERT_EQUAL_STRING("say \\\"hi\\\"", s.c_str());
}

void test_json_escape_backslash() {
    std::string s = EventSerializer::json_escape("a\\b");
    TEST_ASSERT_EQUAL_STRING("a\\\\b", s.c_str());
}

void test_serializer_meta_rssi_battery() {
    EventSerializer ser;
    auto v = json::parse(ser.serialize(make_event()));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT64(-72, (*v)["meta"]["rssi"].as_int());
    TEST_ASSERT_EQUAL_INT64(3800, (*v)["meta"]["battery_mv"].as_int());
}

void test_serialize_no_label_field_omitted() {
    // serialize() (no-label mode) must NOT emit a "label" key per detection.
    EventSerializer ser;
    auto json_str = ser.serialize(make_event());
    auto v = json::parse(json_str);
    TEST_ASSERT_TRUE(v.has_value());
    // The label key must be absent (MiniJson returns Null on miss).
    TEST_ASSERT_TRUE((*v)["detections"][0]["label"].is_null());
}

void test_serializer_no_detections() {
    EventSerializer ser;
    DetectionEvent ev;
    ev.event_ts_ms = 0;
    auto v = json::parse(ser.serialize(ev));
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_UINT(0u, (*v)["detections"].size());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_serializer_produces_valid_json);
    RUN_TEST(test_serializer_schema_field);
    RUN_TEST(test_serializer_event_ts);
    RUN_TEST(test_serializer_one_detection);
    RUN_TEST(test_serializer_with_labels);
    RUN_TEST(test_json_escape_quotes);
    RUN_TEST(test_json_escape_backslash);
    RUN_TEST(test_serializer_meta_rssi_battery);
    RUN_TEST(test_serialize_no_label_field_omitted);
    RUN_TEST(test_serializer_no_detections);
    return UNITY_END();
}
