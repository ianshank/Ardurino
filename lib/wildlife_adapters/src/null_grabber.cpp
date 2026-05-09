#include "wildlife/adapters/null_grabber.hpp"

namespace wildlife {

bool NullGrabber::grab(JpegBuffer& buf) noexcept {
    buf.data.clear();
    return false;
}

} // namespace wildlife