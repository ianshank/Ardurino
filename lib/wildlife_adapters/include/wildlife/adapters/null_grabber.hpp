#pragma once

#include "wildlife/io/iperipheral.hpp"

namespace wildlife {

class NullGrabber final : public IFrameGrabber {
public:
    bool grab(JpegBuffer& buf) noexcept override;
};

} // namespace wildlife