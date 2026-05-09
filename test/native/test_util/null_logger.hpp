#pragma once

// Test utility — NullLogger discards all log output.
// Inject into any class that accepts ILogger* to exercise the non-null
// logger branch in unit tests without producing output.

#include "wildlife/io/ilogger.hpp"

namespace wildlife {
namespace test {

class NullLogger : public ILogger {
  public:
    void log(LogLevel, std::string_view) noexcept override {}
};

} // namespace test
} // namespace wildlife
