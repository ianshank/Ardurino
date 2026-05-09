#pragma once
#include <cstdint>
#include <string_view>

namespace wildlife {

enum class LogLevel : uint8_t { Debug = 0, Info, Warn, Error };

class ILogger {
  public:
    virtual ~ILogger() = default;
    virtual void log(LogLevel level, std::string_view msg) noexcept = 0;

    void debug(std::string_view m) noexcept { log(LogLevel::Debug, m); }
    void info(std::string_view m) noexcept { log(LogLevel::Info, m); }
    void warn(std::string_view m) noexcept { log(LogLevel::Warn, m); }
    void error(std::string_view m) noexcept { log(LogLevel::Error, m); }
};

} // namespace wildlife
