#pragma once

#include <Arduino.h>

#include "wildlife/io/ilogger.hpp"

namespace wildlife {

class SerialLogger final : public ILogger {
public:
    explicit SerialLogger(Stream& stream = Serial,
                          LogLevel min_level = LogLevel::Info) noexcept
        : _stream(&stream), _min_level(min_level) {}

    void set_min_level(LogLevel level) noexcept { _min_level = level; }
    void log(LogLevel level, std::string_view msg) noexcept override;

private:
    Stream*  _stream;
    LogLevel _min_level;
};

} // namespace wildlife