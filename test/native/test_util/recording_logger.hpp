#pragma once

// Test utility — RecordingLogger captures log calls for assertion in tests.

#include "wildlife/io/ilogger.hpp"

#include <string>
#include <vector>

namespace wildlife {
namespace test {

struct LogEntry {
    LogLevel level;
    std::string message;
};

class RecordingLogger : public ILogger {
  public:
    void log(LogLevel level, std::string_view msg) noexcept override {
        _entries.push_back({level, std::string(msg)});
    }

    const std::vector<LogEntry>& entries() const noexcept { return _entries; }

    bool has_level(LogLevel level) const noexcept {
        for (const auto& e : _entries) {
            if (e.level == level)
                return true;
        }
        return false;
    }

    bool has_message(std::string_view substr) const noexcept {
        for (const auto& e : _entries) {
            if (e.message.find(substr) != std::string::npos)
                return true;
        }
        return false;
    }

    void clear() noexcept { _entries.clear(); }

  private:
    std::vector<LogEntry> _entries;
};

} // namespace test
} // namespace wildlife
