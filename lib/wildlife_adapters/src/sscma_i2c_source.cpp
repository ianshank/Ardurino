#include "wildlife/adapters/sscma_i2c_source.hpp"

#include "wildlife/app/inference_filter.hpp"
#include "wildlife/app/log_helpers.hpp"

#include <cstdio>

namespace wildlife {

bool SscmaI2cSource::begin() noexcept {
    // If already healthy, skip re-init.
    if (_began && _healthy) return true;

    // Rate-limit re-init attempts: only retry every 3 seconds to avoid
    // flooding the I2C bus on each loop() iteration.
    if (_began && !_healthy) {
        const uint64_t now = _clock ? _clock->millis() : 0u;
        if (now - _last_begin_attempt_ms < 3000) return false;
        _last_begin_attempt_ms = now;
        log_debug("SscmaI2cSource: retrying begin()...");
    }

    _wire->begin(
        static_cast<int>(_profile.i2c_sda),
        static_cast<int>(_profile.i2c_scl),
        _profile.i2c_freq_hz
    );

    // SSCMA library signature:
    //   bool begin(TwoWire *wire, int32_t rst, uint16_t address,
    //              uint32_t wait_delay, uint32_t clock)
    // Returns true on success.
    const bool rc = _sscma.begin(_wire,
                                 /*rst=*/-1,
                                 static_cast<uint16_t>(_profile.sscma_i2c_addr),
                                 /*wait_delay=*/2,
                                 _profile.i2c_freq_hz);
    if (!rc) {
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "SscmaI2cSource: begin() FAILED — addr=0x%02X sda=%u scl=%u freq=%lu",
                 _profile.sscma_i2c_addr,
                 _profile.i2c_sda,
                 _profile.i2c_scl,
                 (unsigned long)_profile.i2c_freq_hz);
        log_warn(buf);
        _healthy = false;
        _began   = true;
        return false;
    }

    log_debug("SscmaI2cSource: begin() OK — Grove Vision AI V2 connected");
    _healthy = true;
    _began   = true;
    return true;
}

bool SscmaI2cSource::invoke() noexcept {
    if (!_healthy) {
        // Attempt lazy re-init so that hot-plug / late-power is handled.
        if (!begin()) return false;
    }

    const int rc = _sscma.invoke(static_cast<int>(_inference_cfg.sscma_invoke_times),
                                 _inference_cfg.sscma_filter != 0,
                                 _inference_cfg.sscma_encode != 0);
    if (rc != 0) {
        char buf[80];
        snprintf(buf, sizeof(buf), "SscmaI2cSource: invoke() rc=%d", rc);
        log_warn(buf);
        _healthy = false;
        _detections.clear();
        return false;
    }

    _healthy = true;
    _detections.clear();

    const uint64_t now_ms = _clock ? _clock->millis() : 0u;

    for (const auto& box : _sscma.boxes()) {
        if (box.score < _inference_cfg.min_score) continue;

        if (!matches_class_filter(box.target, _inference_cfg.classes_of_interest)) continue;

        Detection d;
        d.class_id  = box.target;
        d.score     = static_cast<uint8_t>(box.score);
        d.bbox      = BBox{
            static_cast<int16_t>(box.x),
            static_cast<int16_t>(box.y),
            static_cast<int16_t>(box.w),
            static_cast<int16_t>(box.h)
        };
        d.ts_ms     = now_ms;
        _detections.push_back(d);
    }

    if (_logger) {
        const auto msg = format_kv("component", "SscmaI2cSource",
                                   "detections", static_cast<unsigned int>(_detections.size()));
        _logger->log(LogLevel::Debug, msg);
    }

    return true;
}

} // namespace wildlife
