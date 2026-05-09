#pragma once

// Wildlife Core — IOtaService
// Abstracts over OTA update mechanics.
// Decouples OTA decision logic (living in core) from Arduino Update/HTTPUpdate.

#include <cstdint>
#include <string>
#include <string_view>

namespace wildlife {

struct OtaConfig {
    std::string url;
    std::string channel;   // "release", "beta", etc.
    uint8_t     check_interval_h{24};
};

enum class OtaCheckResult : uint8_t {
    UpToDate = 0,   // no update available
    Available,      // an update is ready to apply
    Error,          // check failed (network / server error)
};

class IOtaService {
public:
    virtual ~IOtaService() = default;

    // Check whether an update is available.  Non-blocking on failure.
    virtual OtaCheckResult check(const OtaConfig& cfg) noexcept = 0;

    // Apply the update previously found by check().
    // Returns true if the update was written successfully;
    // device must be rebooted afterwards.
    virtual bool apply(const OtaConfig& cfg) noexcept = 0;

    // Mark the current firmware slot as valid (for rollback-capable OTA).
    virtual void mark_valid() noexcept = 0;

    // Firmware version string currently running.
    virtual std::string_view current_version() const noexcept = 0;
};

} // namespace wildlife
