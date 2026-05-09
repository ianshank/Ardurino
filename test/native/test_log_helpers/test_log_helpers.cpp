#include <unity.h>
#include "wildlife/app/log_helpers.hpp"

using namespace wildlife;

void setUp()    {}
void tearDown() {}

void test_parse_debug() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Debug),
                          static_cast<int>(parse_log_level("debug")));
}

void test_parse_info() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Info),
                          static_cast<int>(parse_log_level("info")));
}

void test_parse_warn() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Warn),
                          static_cast<int>(parse_log_level("warn")));
}

void test_parse_error() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Error),
                          static_cast<int>(parse_log_level("error")));
}

void test_parse_uppercase_is_case_insensitive() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Warn),
                          static_cast<int>(parse_log_level("WARN")));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Debug),
                          static_cast<int>(parse_log_level("DEBUG")));
}

void test_parse_mixed_case() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Error),
                          static_cast<int>(parse_log_level("Error")));
}

void test_parse_unknown_falls_back_to_info() {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Info),
                          static_cast<int>(parse_log_level("verbose")));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Info),
                          static_cast<int>(parse_log_level("")));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(LogLevel::Info),
                          static_cast<int>(parse_log_level("trace")));
}

void test_level_name_round_trip() {
    for (auto lvl : {LogLevel::Debug, LogLevel::Info, LogLevel::Warn, LogLevel::Error}) {
        const auto name  = log_level_name(lvl);
        const auto back  = parse_log_level(name);
        TEST_ASSERT_EQUAL_INT(static_cast<int>(lvl), static_cast<int>(back));
    }
}

void test_format_kv_empty() {
    TEST_ASSERT_EQUAL_STRING("", format_kv().c_str());
}

void test_format_kv_single_pair() {
    TEST_ASSERT_EQUAL_STRING("class=3", format_kv("class", 3).c_str());
}

void test_format_kv_two_pairs() {
    auto r = format_kv("class", 3, "score", 90);
    TEST_ASSERT_EQUAL_STRING("class=3 score=90", r.c_str());
}

void test_format_kv_bool_values() {
    TEST_ASSERT_EQUAL_STRING("active=true", format_kv("active", true).c_str());
    TEST_ASSERT_EQUAL_STRING("active=false", format_kv("active", false).c_str());
}

void test_format_kv_string_value_spaces_replaced() {
    auto r = format_kv("msg", std::string("hello world"));
    TEST_ASSERT_EQUAL_STRING("msg=hello_world", r.c_str());
}

void test_format_kv_null_char_ptr() {
    const char* p = nullptr;
    auto r = format_kv("key", p);
    TEST_ASSERT_EQUAL_STRING("key=", r.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_parse_debug);
    RUN_TEST(test_parse_info);
    RUN_TEST(test_parse_warn);
    RUN_TEST(test_parse_error);
    RUN_TEST(test_parse_uppercase_is_case_insensitive);
    RUN_TEST(test_parse_mixed_case);
    RUN_TEST(test_parse_unknown_falls_back_to_info);
    RUN_TEST(test_level_name_round_trip);
    RUN_TEST(test_format_kv_empty);
    RUN_TEST(test_format_kv_single_pair);
    RUN_TEST(test_format_kv_two_pairs);
    RUN_TEST(test_format_kv_bool_values);
    RUN_TEST(test_format_kv_string_value_spaces_replaced);
    RUN_TEST(test_format_kv_null_char_ptr);
    return UNITY_END();
}
