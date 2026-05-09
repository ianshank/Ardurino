#pragma once

// Test utility — FakeFrameGrabber: controllable IFrameGrabber.

#include "wildlife/io/iperipheral.hpp"

#include <cstdint>
#include <vector>

namespace wildlife {
namespace test {

class FakeFrameGrabber : public IFrameGrabber {
  public:
    void set_grab_result(bool ok) noexcept { _ok = ok; }

    void set_image(std::vector<uint8_t> data) noexcept { _image = std::move(data); }

    bool grab(JpegBuffer& buf) noexcept override {
        ++_grab_count;
        if (!_ok)
            return false;
        buf.data = _image;
        return true;
    }

    uint32_t grab_count() const noexcept { return _grab_count; }

  private:
    bool _ok{true};
    std::vector<uint8_t> _image;
    uint32_t _grab_count{0};
};

} // namespace test
} // namespace wildlife
