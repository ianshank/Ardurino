#include "wildlife/app/snapshot_chunker.hpp"

#include <algorithm>
#include <cstring>

namespace wildlife {
namespace app {

// ---------------------------------------------------------------------------
// SHA-256 — portable, no external deps
// ---------------------------------------------------------------------------
namespace {

static constexpr uint32_t kK[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

static uint32_t rotr32(uint32_t x, uint32_t n) noexcept {
    return (x >> n) | (x << (32u - n));
}

struct Sha256State {
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    };
    uint8_t buf[64]{};
    uint64_t bit_len{0};
    uint32_t buf_len{0};

    void process_block(const uint8_t block[64]) noexcept {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            w[i] = (static_cast<uint32_t>(block[i * 4]) << 24) |
                   (static_cast<uint32_t>(block[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(block[i * 4 + 2]) << 8) |
                   static_cast<uint32_t>(block[i * 4 + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }
        uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t tmp1 = hh + S1 + ch + kK[i] + w[i];
            uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t tmp2 = S0 + maj;
            hh = g;
            g = f;
            f = e;
            e = d + tmp1;
            d = c;
            c = b;
            b = a;
            a = tmp1 + tmp2;
        }
        h[0] += a;
        h[1] += b;
        h[2] += c;
        h[3] += d;
        h[4] += e;
        h[5] += f;
        h[6] += g;
        h[7] += hh;
    }

    void update(const uint8_t* data, std::size_t len) noexcept {
        bit_len += len * 8u;
        while (len > 0) {
            std::size_t space = 64 - buf_len;
            std::size_t take = std::min(len, space);
            std::memcpy(buf + buf_len, data, take);
            buf_len += static_cast<uint32_t>(take);
            data += take;
            len -= take;
            if (buf_len == 64) {
                process_block(buf);
                buf_len = 0;
            }
        }
    }

    void finalize(uint8_t digest[32]) noexcept {
        buf[buf_len++] = 0x80;
        if (buf_len > 56) {
            while (buf_len < 64)
                buf[buf_len++] = 0;
            process_block(buf);
            buf_len = 0;
        }
        while (buf_len < 56)
            buf[buf_len++] = 0;
        for (int i = 7; i >= 0; --i) {
            buf[buf_len++] = static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF);
        }
        process_block(buf);
        for (int i = 0; i < 8; ++i) {
            digest[i * 4] = static_cast<uint8_t>((h[i] >> 24) & 0xFF);
            digest[i * 4 + 1] = static_cast<uint8_t>((h[i] >> 16) & 0xFF);
            digest[i * 4 + 2] = static_cast<uint8_t>((h[i] >> 8) & 0xFF);
            digest[i * 4 + 3] = static_cast<uint8_t>((h[i]) & 0xFF);
        }
    }
};

static const char* kHexChars = "0123456789abcdef";

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string sha256_hex(const uint8_t* data, std::size_t len) noexcept {
    Sha256State state;
    state.update(data, len);
    uint8_t digest[32]{};
    state.finalize(digest);
    std::string out(64, '\0');
    for (int i = 0; i < 32; ++i) {
        out[i * 2] = kHexChars[(digest[i] >> 4) & 0xF];
        out[i * 2 + 1] = kHexChars[digest[i] & 0xF];
    }
    return out;
}

std::vector<SnapshotChunk> chunk_jpeg(const JpegBuffer& buf, std::size_t max_bytes,
                                      const std::string& snapshot_id) noexcept {
    if (buf.data.empty() || max_bytes == 0)
        return {};
    const std::size_t total_bytes = buf.data.size();
    const uint32_t n_chunks = static_cast<uint32_t>((total_bytes + max_bytes - 1) / max_bytes);
    std::vector<SnapshotChunk> chunks;
    chunks.reserve(n_chunks);
    for (uint32_t i = 0; i < n_chunks; ++i) {
        SnapshotChunk ch;
        ch.snapshot_id = snapshot_id;
        ch.seq = i;
        ch.total = n_chunks;
        const std::size_t offset = i * max_bytes;
        const std::size_t take = std::min(max_bytes, total_bytes - offset);
        ch.data.assign(buf.data.begin() + static_cast<ptrdiff_t>(offset),
                       buf.data.begin() + static_cast<ptrdiff_t>(offset + take));
        if (i == 0) {
            ch.sha256_hex = sha256_hex(buf.data.data(), total_bytes);
        }
        chunks.push_back(std::move(ch));
    }
    return chunks;
}

JpegBuffer reassemble_chunks(const std::vector<SnapshotChunk>& chunks) noexcept {
    if (chunks.empty())
        return {};
    std::size_t total = 0;
    for (const auto& c : chunks)
        total += c.data.size();
    JpegBuffer out;
    out.data.reserve(total);
    for (const auto& c : chunks) {
        out.data.insert(out.data.end(), c.data.begin(), c.data.end());
    }
    return out;
}

} // namespace app
} // namespace wildlife
