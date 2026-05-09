#include "wildlife/adapters/serial_logger.hpp"

#include "wildlife/app/log_helpers.hpp"

namespace wildlife {

void SerialLogger::log(LogLevel level, std::string_view msg) noexcept {
    if (!_stream || static_cast<uint8_t>(level) < static_cast<uint8_t>(_min_level)) {
        return;
    }

    _stream->print(F("[wildlife]["));
    const auto level_name = log_level_name(level);
    _stream->write(reinterpret_cast<const uint8_t*>(level_name.data()), level_name.size());
    _stream->print(F("] "));
    _stream->write(reinterpret_cast<const uint8_t*>(msg.data()), msg.size());
    _stream->println();
}

} // namespace wildlife