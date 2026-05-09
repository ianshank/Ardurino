#pragma once

// Wildlife Spotter — Core
// Single place that records the firmware version string passed in from the
// build system. Lives in Core so host tests and target firmware share it.

namespace wildlife {

#ifndef WILDLIFE_FW_VERSION
#define WILDLIFE_FW_VERSION "0.0.0-dev"
#endif

constexpr const char* kFirmwareVersion = WILDLIFE_FW_VERSION;

} // namespace wildlife
