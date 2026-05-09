#pragma once

// SscmaI2cSource — IInferenceSource implementation backed by the Seeed SSCMA
// library communicating with the Grove Vision AI V2 over I²C.
//
// This file is target-only (Arduino/ESP-IDF); it must NOT be compiled in the
// native (host-test) environment.

#include "wildlife/domain/board_profile.hpp"
#include "wildlife/domain/runtime_config.hpp"
#include "wildlife/io/iclock.hpp"
#include "wildlife/io/ilogger.hpp"
#include "wildlife/io/iperipheral.hpp"

#include <Seeed_Arduino_SSCMA.h>
#include <Wire.h>

namespace wildlife {

class SscmaI2cSource final : public IInferenceSource {
  public:
    // -----------------------------------------------------------------------
    // Constructor
    // wire         — I²C bus (e.g. Wire)
    // profile      — board pinmap; provides i2c_sda/scl/freq_hz/sscma_i2c_addr
    // inference_cfg — min_score, poll_interval_ms, classes_of_interest
    // clock        — monotonic time source for Detection::ts_ms (optional)
    // logger       — diagnostic logging (optional)
    // -----------------------------------------------------------------------
    SscmaI2cSource(TwoWire& wire, const BoardProfile& profile,
                   const RuntimeConfig::Inference& inference_cfg, IClock* clock = nullptr,
                   ILogger* logger = nullptr) noexcept
        : _wire(&wire), _profile(profile), _inference_cfg(inference_cfg), _clock(clock),
          _logger(logger) {}

    // -----------------------------------------------------------------------
    // IInferenceSource
    // -----------------------------------------------------------------------

    /// Initialise the SSCMA library and the underlying I²C device.
    /// Must be called once before invoke(). Returns true on success.
    bool begin() noexcept;

    /// Run one inference cycle. Fills the internal detection list.
    bool invoke() noexcept override;

    /// Detections from the last successful invoke().
    const std::vector<Detection>& detections() const noexcept override { return _detections; }

    /// True when the hardware has been successfully initialised and the
    /// last invoke() did not return a fatal error.
    bool healthy() const noexcept override { return _healthy; }

    /// Direct access to the underlying SSCMA instance (e.g. for SscmaJpegGrabber).
    SSCMA& sscma() noexcept { return _sscma; }

  private:
    static constexpr uint64_t kBeginRetryIntervalMs = 3000;

    void log_debug(const char* msg) const noexcept {
        if (_logger)
            _logger->log(LogLevel::Debug, msg);
    }
    void log_warn(const char* msg) const noexcept {
        if (_logger)
            _logger->log(LogLevel::Warn, msg);
    }

    TwoWire* _wire;
    BoardProfile _profile;
    RuntimeConfig::Inference _inference_cfg;
    IClock* _clock;
    ILogger* _logger;

    SSCMA _sscma;
    std::vector<Detection> _detections;
    bool _healthy{false};
    bool _began{false};
    uint64_t _last_begin_attempt_ms{0};
};

} // namespace wildlife
