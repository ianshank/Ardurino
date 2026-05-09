#pragma once

// Wildlife Core — SnapshotChunker
// Splits a JpegBuffer into fixed-size SnapshotChunks suitable for MQTT.
// The first chunk (seq==0) carries a SHA-256 hex digest of the full JPEG.

#include "wildlife/domain/detection.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace wildlife {
namespace app {

struct SnapshotChunk {
    std::string snapshot_id;
    uint32_t seq{0};
    uint32_t total{0}; // total number of chunks
    std::vector<uint8_t> data;
    std::string sha256_hex; // only on seq==0; empty otherwise
};

// Chunk a JPEG buffer into SnapshotChunks.
// max_bytes: maximum payload bytes per chunk (data field only).
// snapshot_id: opaque ID carried in each chunk.
// Returns empty vector if buf.data is empty or max_bytes == 0.
std::vector<SnapshotChunk> chunk_jpeg(const JpegBuffer& buf, std::size_t max_bytes,
                                      const std::string& snapshot_id) noexcept;

// Reassemble SnapshotChunks back into a JpegBuffer.
// Chunks must be sorted by seq. Returns empty JpegBuffer on error.
JpegBuffer reassemble_chunks(const std::vector<SnapshotChunk>& chunks) noexcept;

// Compute SHA-256 of data; return lowercase hex string (64 chars).
std::string sha256_hex(const uint8_t* data, std::size_t len) noexcept;

} // namespace app
} // namespace wildlife
