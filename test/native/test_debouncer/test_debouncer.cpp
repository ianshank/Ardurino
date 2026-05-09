#include "../test_util/null_logger.hpp"
#include "wildlife/app/event_debouncer.hpp"

#include <unity.h>

using namespace wildlife::app;
using wildlife::test::NullLogger;

void setUp() {}
void tearDown() {}

void test_first_emit_allowed() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 1000}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 1000));
}

void test_second_emit_suppressed_within_window() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 5000}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 1000));
    TEST_ASSERT_FALSE(d.should_emit(0, "dog", 3000)); // 2s < 5s
}

void test_emit_allowed_after_window() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 5000}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 1000));
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 7000)); // 6s > 5s
}

void test_different_class_ids_independent() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 5000}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 1000));
    TEST_ASSERT_TRUE(d.should_emit(1, "cat", 1500));  // different id
    TEST_ASSERT_FALSE(d.should_emit(0, "dog", 2000)); // still debouncing
}

void test_reset_clears_state() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 5000}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 1000));
    TEST_ASSERT_FALSE(d.should_emit(0, "dog", 2000));
    d.reset();
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 2000)); // fresh start
}

void test_allow_list_empty_accepts_all() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 100, .classes_of_interest = {}}};
    TEST_ASSERT_TRUE(d.should_emit(5, "anything", 0));
}

void test_allow_list_filters() {
    EventDebouncer d{DebouncerConfig{.debounce_ms = 100, .classes_of_interest = {"dog", "cat"}}};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 0));
    TEST_ASSERT_FALSE(d.should_emit(1, "horse", 0)); // not in list
}

void test_with_logger_suppressed_logs() {
    // Exercises the non-null ILogger branch in both suppression paths.
    NullLogger log;
    EventDebouncer d{DebouncerConfig{.debounce_ms = 5000}, &log};
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 0));
    TEST_ASSERT_FALSE(d.should_emit(0, "dog", 100)); // suppress — logs debug
    d.reset();                                       // logs debug
    TEST_ASSERT_TRUE(d.should_emit(0, "dog", 200));

    // allow-list rejection with logger
    EventDebouncer d2{DebouncerConfig{.debounce_ms = 100, .classes_of_interest = {"cat"}}, &log};
    TEST_ASSERT_FALSE(d2.should_emit(0, "dog", 0)); // not in list — logs debug
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_first_emit_allowed);
    RUN_TEST(test_second_emit_suppressed_within_window);
    RUN_TEST(test_emit_allowed_after_window);
    RUN_TEST(test_different_class_ids_independent);
    RUN_TEST(test_reset_clears_state);
    RUN_TEST(test_allow_list_empty_accepts_all);
    RUN_TEST(test_allow_list_filters);
    RUN_TEST(test_with_logger_suppressed_logs);
    return UNITY_END();
}
