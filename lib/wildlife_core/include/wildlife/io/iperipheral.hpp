#pragma once
#include "wildlife/domain/detection.hpp"

#include <vector>

namespace wildlife {

class IInferenceSource {
  public:
    virtual ~IInferenceSource() = default;
    // Runs one inference cycle. Returns true on success.
    virtual bool invoke() noexcept = 0;
    // Detections from the last successful invoke().
    virtual const std::vector<Detection>& detections() const noexcept = 0;
    // True if the hardware is connected and responsive.
    virtual bool healthy() const noexcept = 0;
};

class IFrameGrabber {
  public:
    virtual ~IFrameGrabber() = default;
    // Fills buf with the JPEG of the last inference frame. Returns true on success.
    virtual bool grab(JpegBuffer& buf) noexcept = 0;
};

} // namespace wildlife
