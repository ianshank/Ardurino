#include "wildlife/json/mini_json.hpp"

#include <unity.h>

using namespace wildlife::json;

void setUp() {}
void tearDown() {}

// --- parse ---

void test_parse_null() {
    auto v = parse("null");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(JType::Null), static_cast<int>(v->type()));
}

void test_parse_bool_true() {
    auto v = parse("true");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_TRUE(v->as_bool());
}

void test_parse_bool_false() {
    auto v = parse("false");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_FALSE(v->as_bool());
}

void test_parse_int() {
    auto v = parse("42");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT64(42, v->as_int());
}

void test_parse_negative_int() {
    auto v = parse("-7");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT64(-7, v->as_int());
}

void test_parse_double() {
    auto v = parse("3.14");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(JType::Double), static_cast<int>(v->type()));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 3.14, v->as_double());
}

void test_parse_string() {
    auto v = parse("\"hello\"");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("hello", std::string(v->as_str()).c_str());
}

void test_parse_empty_string() {
    auto v = parse("\"\"");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_STRING("", std::string(v->as_str()).c_str());
}

void test_parse_string_escape_newline() {
    auto v = parse("\"line1\\nline2\"");
    TEST_ASSERT_TRUE(v.has_value());
    std::string s(v->as_str());
    TEST_ASSERT_EQUAL_INT('\n', s[5]);
}

void test_parse_empty_object() {
    auto v = parse("{}");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(JType::Object), static_cast<int>(v->type()));
    TEST_ASSERT_EQUAL_UINT(0u, v->size());
}

void test_parse_flat_object() {
    auto v = parse("{\"a\":1,\"b\":true,\"c\":\"x\"}");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT64(1, (*v)["a"].as_int());
    TEST_ASSERT_TRUE((*v)["b"].as_bool());
    TEST_ASSERT_EQUAL_STRING("x", std::string((*v)["c"].as_str()).c_str());
}

void test_parse_nested_object() {
    auto v = parse("{\"outer\":{\"inner\":99}}");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT64(99, (*v)["outer"]["inner"].as_int());
}

void test_parse_empty_array() {
    auto v = parse("[]");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_INT(static_cast<int>(JType::Array), static_cast<int>(v->type()));
    TEST_ASSERT_EQUAL_UINT(0u, v->size());
}

void test_parse_array_elements() {
    auto v = parse("[1,2,3]");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_EQUAL_UINT(3u, v->size());
    TEST_ASSERT_EQUAL_INT64(2, (*v)[1].as_int());
}

void test_missing_key_returns_null() {
    auto v = parse("{\"a\":1}");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_TRUE((*v)["missing"].is_null());
}

void test_oob_array_returns_null() {
    auto v = parse("[1]");
    TEST_ASSERT_TRUE(v.has_value());
    TEST_ASSERT_TRUE((*v)[99].is_null());
}

void test_trailing_garbage_fails() {
    auto v = parse("1 extra");
    TEST_ASSERT_FALSE(v.has_value());
}

void test_malformed_fails() {
    TEST_ASSERT_FALSE(parse("{\"a\":").has_value());
    TEST_ASSERT_FALSE(parse("[1,2,").has_value());
    TEST_ASSERT_FALSE(parse("\"unterminated").has_value());
}

void test_empty_input_fails() {
    TEST_ASSERT_FALSE(parse("").has_value());
}

// --- stringify round-trip ---

void test_stringify_roundtrip_flat_object() {
    const std::string json_str = "{\"k\":123}";
    auto v = parse(json_str);
    TEST_ASSERT_TRUE(v.has_value());
    auto s = stringify(*v);
    TEST_ASSERT_EQUAL_STRING(json_str.c_str(), s.c_str());
}

// --- mutation ---

void test_set_overwrites() {
    auto v = parse("{\"x\":1}");
    TEST_ASSERT_TRUE(v.has_value());
    v->set("x", JVal{static_cast<int64_t>(42)});
    TEST_ASSERT_EQUAL_INT64(42, (*v)["x"].as_int());
}

void test_remove_key() {
    auto v = parse("{\"a\":1,\"b\":2}");
    TEST_ASSERT_TRUE(v.has_value());
    v->remove("a");
    TEST_ASSERT_TRUE((*v)["a"].is_null());
    TEST_ASSERT_EQUAL_UINT(1u, v->size());
}

void test_push_back_array() {
    JVal arr = JVal::make_array();
    arr.push_back(JVal{static_cast<int64_t>(10)});
    arr.push_back(JVal{static_cast<int64_t>(20)});
    TEST_ASSERT_EQUAL_UINT(2u, arr.size());
    TEST_ASSERT_EQUAL_INT64(20, arr[1].as_int());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_parse_null);
    RUN_TEST(test_parse_bool_true);
    RUN_TEST(test_parse_bool_false);
    RUN_TEST(test_parse_int);
    RUN_TEST(test_parse_negative_int);
    RUN_TEST(test_parse_double);
    RUN_TEST(test_parse_string);
    RUN_TEST(test_parse_empty_string);
    RUN_TEST(test_parse_string_escape_newline);
    RUN_TEST(test_parse_empty_object);
    RUN_TEST(test_parse_flat_object);
    RUN_TEST(test_parse_nested_object);
    RUN_TEST(test_parse_empty_array);
    RUN_TEST(test_parse_array_elements);
    RUN_TEST(test_missing_key_returns_null);
    RUN_TEST(test_oob_array_returns_null);
    RUN_TEST(test_trailing_garbage_fails);
    RUN_TEST(test_malformed_fails);
    RUN_TEST(test_empty_input_fails);
    RUN_TEST(test_stringify_roundtrip_flat_object);
    RUN_TEST(test_set_overwrites);
    RUN_TEST(test_remove_key);
    RUN_TEST(test_push_back_array);
    return UNITY_END();
}
