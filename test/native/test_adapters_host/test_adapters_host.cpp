// test_adapters_host.cpp
//
// Tests logic-only paths of adapter interfaces using FakeConfigStore.
// Full LittleFsConfigStore requires hardware and is validated via HIL only.

#include <unity.h>
#include "../test_util/fake_config_store.hpp"

using namespace wildlife::test;

void setUp()    {}
void tearDown() {}

// ---------------------------------------------------------------------------
// IConfigStore contract — satisfied by FakeConfigStore (documents expected
// behaviour that LittleFsConfigStore must also honour).
// ---------------------------------------------------------------------------

void test_store_write_then_read_round_trip() {
    FakeConfigStore store;
    TEST_ASSERT_TRUE(store.write("/cfg/test.json", R"({"key":"value"})"));

    std::string out;
    TEST_ASSERT_TRUE(store.read("/cfg/test.json", out));
    TEST_ASSERT_EQUAL_STRING(R"({"key":"value"})", out.c_str());
}

void test_store_exists_true_after_write() {
    FakeConfigStore store;
    store.write("/a.json", "{}");
    TEST_ASSERT_TRUE(store.exists("/a.json"));
}

void test_store_exists_false_for_missing_path() {
    FakeConfigStore store;
    TEST_ASSERT_FALSE(store.exists("/no_such.json"));
}

void test_store_read_fails_for_missing_path() {
    FakeConfigStore store;
    std::string out;
    TEST_ASSERT_FALSE(store.read("/missing.json", out));
}

void test_store_remove_clears_entry() {
    FakeConfigStore store;
    store.write("/b.json", "data");
    TEST_ASSERT_TRUE(store.remove("/b.json"));
    TEST_ASSERT_FALSE(store.exists("/b.json"));
}

void test_store_overwrite_updates_content() {
    FakeConfigStore store;
    store.write("/c.json", "v1");
    store.write("/c.json", "v2");
    std::string out;
    store.read("/c.json", out);
    TEST_ASSERT_EQUAL_STRING("v2", out.c_str());
}

void test_store_remove_returns_false_when_missing() {
    FakeConfigStore store;
    TEST_ASSERT_FALSE(store.remove("/nonexistent.json"));
}

void test_store_seed_populates_entry() {
    FakeConfigStore store;
    store.seed("/seeded.json", R"({"seeded":true})");
    std::string out;
    TEST_ASSERT_TRUE(store.read("/seeded.json", out));
    TEST_ASSERT_EQUAL_STRING(R"({"seeded":true})", out.c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_store_write_then_read_round_trip);
    RUN_TEST(test_store_exists_true_after_write);
    RUN_TEST(test_store_exists_false_for_missing_path);
    RUN_TEST(test_store_read_fails_for_missing_path);
    RUN_TEST(test_store_remove_clears_entry);
    RUN_TEST(test_store_overwrite_updates_content);
    RUN_TEST(test_store_remove_returns_false_when_missing);
    RUN_TEST(test_store_seed_populates_entry);
    return UNITY_END();
}
