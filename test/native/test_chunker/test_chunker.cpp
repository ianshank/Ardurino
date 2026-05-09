#include <unity.h>
#include "wildlife/app/snapshot_chunker.hpp"

using namespace wildlife;
using namespace wildlife::app;

void setUp()    {}
void tearDown() {}

static JpegBuffer make_buf(std::size_t n, uint8_t fill = 0xAA) {
    JpegBuffer b;
    b.data.assign(n, fill);
    return b;
}

void test_empty_buf_returns_empty() {
    JpegBuffer empty;
    auto chunks = chunk_jpeg(empty, 64, "id");
    TEST_ASSERT_EQUAL_UINT(0u, chunks.size());
}

void test_zero_max_returns_empty() {
    auto chunks = chunk_jpeg(make_buf(100), 0, "id");
    TEST_ASSERT_EQUAL_UINT(0u, chunks.size());
}

void test_single_chunk_small_buf() {
    auto buf    = make_buf(50);
    auto chunks = chunk_jpeg(buf, 64, "snap1");
    TEST_ASSERT_EQUAL_UINT(1u, chunks.size());
    TEST_ASSERT_EQUAL_UINT(1u, chunks[0].total);
    TEST_ASSERT_EQUAL_UINT(0u, chunks[0].seq);
    TEST_ASSERT_EQUAL_UINT(50u, chunks[0].data.size());
    TEST_ASSERT_FALSE(chunks[0].sha256_hex.empty());
}

void test_multi_chunk() {
    auto buf    = make_buf(100);
    auto chunks = chunk_jpeg(buf, 40, "snap2");
    TEST_ASSERT_EQUAL_UINT(3u, chunks.size());
    TEST_ASSERT_EQUAL_UINT(0u, chunks[0].seq);
    TEST_ASSERT_EQUAL_UINT(1u, chunks[1].seq);
    TEST_ASSERT_EQUAL_UINT(2u, chunks[2].seq);
    // total sizes: 40 + 40 + 20 = 100
    TEST_ASSERT_EQUAL_UINT(40u, chunks[0].data.size());
    TEST_ASSERT_EQUAL_UINT(40u, chunks[1].data.size());
    TEST_ASSERT_EQUAL_UINT(20u, chunks[2].data.size());
    // sha256 only on seq 0
    TEST_ASSERT_FALSE(chunks[0].sha256_hex.empty());
    TEST_ASSERT_TRUE (chunks[1].sha256_hex.empty());
}

void test_exact_boundary() {
    auto buf    = make_buf(64);
    auto chunks = chunk_jpeg(buf, 64, "x");
    TEST_ASSERT_EQUAL_UINT(1u, chunks.size());
}

void test_sha256_hex_length() {
    auto buf    = make_buf(10, 0xFF);
    auto chunks = chunk_jpeg(buf, 1024, "id");
    TEST_ASSERT_EQUAL_UINT(64u, chunks[0].sha256_hex.size());
}

void test_sha256_known_value() {
    // SHA-256 of empty string
    uint8_t empty_data[] = {};
    std::string h = sha256_hex(empty_data, 0);
    TEST_ASSERT_EQUAL_STRING(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
        h.c_str());
}

void test_reassemble_roundtrip() {
    auto buf    = make_buf(100, 0xBB);
    auto chunks = chunk_jpeg(buf, 30, "rid");
    auto rebuilt = reassemble_chunks(chunks);
    TEST_ASSERT_EQUAL_UINT(buf.data.size(), rebuilt.data.size());
    TEST_ASSERT_EQUAL_MEMORY(buf.data.data(), rebuilt.data.data(), buf.data.size());
}

void test_reassemble_empty() {
    std::vector<SnapshotChunk> empty;
    auto r = reassemble_chunks(empty);
    TEST_ASSERT_EQUAL_UINT(0u, r.data.size());
}

void test_snapshot_id_preserved() {
    auto buf    = make_buf(10);
    auto chunks = chunk_jpeg(buf, 5, "myid");
    for (const auto& c : chunks) {
        TEST_ASSERT_EQUAL_STRING("myid", c.snapshot_id.c_str());
    }
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_empty_buf_returns_empty);
    RUN_TEST(test_zero_max_returns_empty);
    RUN_TEST(test_single_chunk_small_buf);
    RUN_TEST(test_multi_chunk);
    RUN_TEST(test_exact_boundary);
    RUN_TEST(test_sha256_hex_length);
    RUN_TEST(test_sha256_known_value);
    RUN_TEST(test_reassemble_roundtrip);
    RUN_TEST(test_reassemble_empty);
    RUN_TEST(test_snapshot_id_preserved);
    return UNITY_END();
}
