#include <unity.h>

#include "wildlife/semver.hpp"
#include "wildlife/version.hpp"

using wildlife::parse_semver;
using wildlife::SemVer;

void setUp() {}
void tearDown() {}

void test_parses_basic_triple() {
    SemVer v{};
    TEST_ASSERT_TRUE(parse_semver("1.2.3", v));
    TEST_ASSERT_EQUAL_UINT16(1, v.major);
    TEST_ASSERT_EQUAL_UINT16(2, v.minor);
    TEST_ASSERT_EQUAL_UINT16(3, v.patch);
}

void test_strips_prerelease_suffix() {
    SemVer v{};
    TEST_ASSERT_TRUE(parse_semver("0.1.0-dev", v));
    TEST_ASSERT_EQUAL_UINT16(0, v.major);
    TEST_ASSERT_EQUAL_UINT16(1, v.minor);
    TEST_ASSERT_EQUAL_UINT16(0, v.patch);
}

void test_rejects_missing_components() {
    SemVer v{99, 99, 99};
    TEST_ASSERT_FALSE(parse_semver("1.2", v));
    TEST_ASSERT_FALSE(parse_semver("1", v));
    TEST_ASSERT_FALSE(parse_semver("", v));
    // out is preserved on failure
    TEST_ASSERT_EQUAL_UINT16(99, v.major);
}

void test_rejects_non_numeric() {
    SemVer v{};
    TEST_ASSERT_FALSE(parse_semver("a.b.c", v));
    TEST_ASSERT_FALSE(parse_semver("1.2.x", v));
    TEST_ASSERT_FALSE(parse_semver("1..3", v));
}

void test_rejects_overflow() {
    SemVer v{};
    TEST_ASSERT_FALSE(parse_semver("65536.0.0", v));
}

void test_firmware_version_constant_parses() {
    SemVer v{};
    TEST_ASSERT_TRUE(parse_semver(wildlife::kFirmwareVersion, v));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parses_basic_triple);
    RUN_TEST(test_strips_prerelease_suffix);
    RUN_TEST(test_rejects_missing_components);
    RUN_TEST(test_rejects_non_numeric);
    RUN_TEST(test_rejects_overflow);
    RUN_TEST(test_firmware_version_constant_parses);
    return UNITY_END();
}
