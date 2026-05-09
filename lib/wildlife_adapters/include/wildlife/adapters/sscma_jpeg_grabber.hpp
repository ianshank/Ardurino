#pragma once

// SscmaJpegGrabber — IFrameGrabber backed by the SSCMA library's last-image buffer.
// Shares the SSCMA instance already owned by SscmaI2cSource.

#include <Seeed_Arduino_SSCMA.h>

#include "wildlife/io/ilogger.hpp"
#include "wildlife/io/iperipheral.hpp"

namespace wildlife {

class SscmaJpegGrabber final : public IFrameGrabber {
public:
    // sscma   — shared SSCMA instance (must outlive this object)
    // logger  — optional diagnostic logger
    explicit SscmaJpegGrabber(SSCMA& sscma, ILogger* logger = nullptr) noexcept
        : _sscma(&sscma), _logger(logger) {}

    // Copies the last encoded JPEG image from the SSCMA library into buf.
    // Returns false when the image buffer is empty (invoke() not yet called,
    // or the model did not encode an image in the last cycle).
    bool grab(JpegBuffer& buf) noexcept override;

private:
    SSCMA*   _sscma;
    ILogger* _logger;
};

} // namespace wildlife
