#include <unity.h>
#include "wildlife/app/inference_filter.hpp"

using wildlife::matches_class_filter;

void setUp() {}
void tearDown() {}

// Empty filter list: everything passes
void test_empty_filter_accepts_any_class() {
    std::vector<std::string> empty;
    TEST_ASSERT_TRUE(matches_class_filter(0, empty));
    TEST_ASSERT_TRUE(matches_class_filter(42, empty));
    TEST_ASSERT_TRUE(matches_class_filter(-1, empty));
}

// Class present in filter: accepted
void test_matching_class_accepted() {
    std::vector<std::string> f{"3"};
    TEST_ASSERT_TRUE(matches_class_filter(3, f));
}

// Class absent from filter: rejected
void test_non_matching_class_rejected() {
    std::vector<std::string> f{"3"};
    TEST_ASSERT_FALSE(matches_class_filter(4, f));
}

// Multiple classes, one matches
void test_multi_filter_partial_match() {
    std::vector<std::string> f{"1", "3", "5"};
    TEST_ASSERT_TRUE(matches_class_filter(3, f));
    TEST_ASSERT_FALSE(matches_class_filter(2, f));
}

// Negative class ID
void test_negative_class_id_in_filter() {
    std::vector<std::string> f{"-1"};
    TEST_ASSERT_TRUE(matches_class_filter(-1, f));
    TEST_ASSERT_FALSE(matches_class_filter(1, f));
}

// Numeric string prefix must not match (e.g. "1" should not accept class 10)
void test_no_prefix_match() {
    std::vector<std::string> f{"1"};
    TEST_ASSERT_FALSE(matches_class_filter(10, f));
    TEST_ASSERT_FALSE(matches_class_filter(100, f));
}

// Zero class ID
void test_class_zero_in_filter() {
    std::vector<std::string> f{"0"};
    TEST_ASSERT_TRUE(matches_class_filter(0, f));
    TEST_ASSERT_FALSE(matches_class_filter(1, f));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_filter_accepts_any_class);
    RUN_TEST(test_matching_class_accepted);
    RUN_TEST(test_non_matching_class_rejected);
    RUN_TEST(test_multi_filter_partial_match);
    RUN_TEST(test_negative_class_id_in_filter);
    RUN_TEST(test_no_prefix_match);
    RUN_TEST(test_class_zero_in_filter);
    return UNITY_END();
}
