#include "../test_util/null_logger.hpp"
#include "wildlife/app/json_label_resolver.hpp"
#include "wildlife/io/iconfig_store.hpp"

#include <string>
#include <unity.h>
#include <unordered_map>

using namespace wildlife;
using namespace wildlife::app;
using wildlife::test::NullLogger;

void setUp() {}
void tearDown() {}

// In-memory IConfigStore stub for tests.
class MemStore : public IConfigStore {
  public:
    std::unordered_map<std::string, std::string> files;

    bool read(const std::string& path, std::string& out) noexcept override {
        auto it = files.find(path);
        if (it == files.end())
            return false;
        out = it->second;
        return true;
    }
    bool write(const std::string& path, const std::string& data) noexcept override {
        files[path] = data;
        return true;
    }
    bool exists(const std::string& path) noexcept override { return files.count(path) > 0; }
    bool remove(const std::string& path) noexcept override { return files.erase(path) > 0; }
};

static const std::string kLabels = R"({"0":"person","1":"cat","2":"dog"})";

void test_load_success() {
    MemStore store;
    store.files["/labels.json"] = kLabels;
    JsonLabelResolver r;
    TEST_ASSERT_TRUE(r.load(store, "/labels.json"));
    TEST_ASSERT_TRUE(r.loaded());
}

void test_known_label() {
    MemStore store;
    store.files["/labels.json"] = kLabels;
    JsonLabelResolver r;
    r.load(store, "/labels.json");
    TEST_ASSERT_EQUAL_STRING("dog", r.name(2).c_str());
}

void test_unknown_label_fallback() {
    MemStore store;
    store.files["/labels.json"] = kLabels;
    JsonLabelResolver r;
    r.load(store, "/labels.json");
    TEST_ASSERT_EQUAL_STRING("class_99", r.name(99).c_str());
}

void test_missing_file_returns_false() {
    MemStore store;
    JsonLabelResolver r;
    TEST_ASSERT_FALSE(r.load(store, "/missing.json"));
    TEST_ASSERT_FALSE(r.loaded());
}

void test_malformed_json_returns_false() {
    MemStore store;
    store.files["/bad.json"] = "{ not valid json!!!";
    JsonLabelResolver r;
    TEST_ASSERT_FALSE(r.load(store, "/bad.json"));
}

void test_hash_populated_on_load() {
    MemStore store;
    store.files["/labels.json"] = kLabels;
    JsonLabelResolver r;
    r.load(store, "/labels.json");
    TEST_ASSERT_EQUAL_UINT(64u, r.hash().size());
}

void test_non_integer_keys_skipped() {
    MemStore store;
    store.files["/labels.json"] = R"({"abc":"ignored","0":"person"})";
    JsonLabelResolver r;
    TEST_ASSERT_TRUE(r.load(store, "/labels.json"));
    TEST_ASSERT_EQUAL_STRING("person", r.name(0).c_str());
    TEST_ASSERT_EQUAL_STRING("class_1", r.name(1).c_str());
}

void test_loaded_true_even_when_all_keys_non_integer() {
    // G5 fix: loaded() must reflect that load() succeeded, even if all
    // label entries were skipped due to non-integer keys.
    MemStore store;
    store.files["/labels.json"] = R"({"abc":"ignored","xyz":"skipped"})";
    JsonLabelResolver r;
    TEST_ASSERT_TRUE(r.load(store, "/labels.json"));
    TEST_ASSERT_TRUE(r.loaded()); // was broken before _loaded flag fix
}

void test_with_logger_error_path() {
    // Exercises the non-null ILogger branch on read failure and parse failure.
    NullLogger log;
    JsonLabelResolver r{&log};
    MemStore store;
    // Missing file — logs error
    TEST_ASSERT_FALSE(r.load(store, "/missing.json"));
    TEST_ASSERT_FALSE(r.loaded());
    // Malformed JSON — logs error
    store.files["/bad.json"] = "not json";
    TEST_ASSERT_FALSE(r.load(store, "/bad.json"));
}

void test_with_logger_success_path() {
    NullLogger log;
    JsonLabelResolver r{&log};
    MemStore store;
    store.files["/labels.json"] = kLabels;
    TEST_ASSERT_TRUE(r.load(store, "/labels.json")); // logs info
    TEST_ASSERT_TRUE(r.loaded());
    TEST_ASSERT_EQUAL_STRING("cat", r.name(1).c_str());
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_load_success);
    RUN_TEST(test_known_label);
    RUN_TEST(test_unknown_label_fallback);
    RUN_TEST(test_missing_file_returns_false);
    RUN_TEST(test_malformed_json_returns_false);
    RUN_TEST(test_hash_populated_on_load);
    RUN_TEST(test_non_integer_keys_skipped);
    RUN_TEST(test_loaded_true_even_when_all_keys_non_integer);
    RUN_TEST(test_with_logger_error_path);
    RUN_TEST(test_with_logger_success_path);
    return UNITY_END();
}
