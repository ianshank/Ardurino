#include "wildlife/app/retry_policy.hpp"

#include <unity.h>

using namespace wildlife::app;

void setUp() {}
void tearDown() {}

void test_zero_attempt_returns_base_ms() {
    RetryPolicy p{
        RetryConfig{.base_ms = 1000, .max_ms = 30000, .max_attempts = 5, .jitter_pct = 0}};
    TEST_ASSERT_EQUAL_UINT32(1000u, p.delay_ms(0));
}

void test_backoff_doubles_each_attempt() {
    RetryPolicy p{
        RetryConfig{.base_ms = 500, .max_ms = 100000, .max_attempts = 10, .jitter_pct = 0}};
    TEST_ASSERT_EQUAL_UINT32(500u, p.delay_ms(0));
    TEST_ASSERT_EQUAL_UINT32(1000u, p.delay_ms(1));
    TEST_ASSERT_EQUAL_UINT32(2000u, p.delay_ms(2));
    TEST_ASSERT_EQUAL_UINT32(4000u, p.delay_ms(3));
}

void test_backoff_capped_at_max() {
    RetryPolicy p{
        RetryConfig{.base_ms = 1000, .max_ms = 5000, .max_attempts = 10, .jitter_pct = 0}};
    TEST_ASSERT_EQUAL_UINT32(5000u, p.delay_ms(10));
    TEST_ASSERT_EQUAL_UINT32(5000u, p.delay_ms(20));
}

void test_jitter_within_bounds() {
    RetryPolicy p{
        RetryConfig{.base_ms = 1000, .max_ms = 30000, .max_attempts = 10, .jitter_pct = 25}};
    // At attempt=0, base=1000, jitter ≤250ms, so result in [750, 1250]
    for (uint32_t entropy = 0; entropy < 1000; ++entropy) {
        uint32_t d = p.delay_ms(0, entropy);
        TEST_ASSERT_GREATER_OR_EQUAL(750u, d);
        TEST_ASSERT_LESS_OR_EQUAL(1250u, d);
    }
}

void test_should_retry_within_limit() {
    RetryPolicy p{RetryConfig{.max_attempts = 3}};
    TEST_ASSERT_TRUE(p.should_retry(0));
    TEST_ASSERT_TRUE(p.should_retry(2));
    TEST_ASSERT_FALSE(p.should_retry(3));
    TEST_ASSERT_FALSE(p.should_retry(100));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_zero_attempt_returns_base_ms);
    RUN_TEST(test_backoff_doubles_each_attempt);
    RUN_TEST(test_backoff_capped_at_max);
    RUN_TEST(test_jitter_within_bounds);
    RUN_TEST(test_should_retry_within_limit);
    return UNITY_END();
}
