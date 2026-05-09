#include "wildlife/adapters/sscma_jpeg_grabber.hpp"

#include "wildlife/app/log_helpers.hpp"

#include <cstdio>

namespace wildlife {

bool SscmaJpegGrabber::grab(JpegBuffer& buf) noexcept {
    const auto& img = _sscma->last_image();

    if (img.isEmpty()) {
        if (_logger) _logger->log(LogLevel::Debug,
                                  "SscmaJpegGrabber: last_image empty — no frame");
        return false;
    }

    buf.data.assign(img.begin(), img.end());

    if (_logger) {
        const auto msg = format_kv("component", "SscmaJpegGrabber",
                                   "bytes", static_cast<unsigned int>(buf.data.size()));
        _logger->log(LogLevel::Debug, msg);
    }

    return true;
}

} // namespace wildlife
