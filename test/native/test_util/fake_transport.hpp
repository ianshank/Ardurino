#pragma once

// Test utility — FakeTransport: records publishes for assertion in tests.

#include "wildlife/io/ipower_transport.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace wildlife {
namespace test {

struct PublishRecord {
    std::string topic;
    std::vector<uint8_t> payload;
    uint8_t qos;
    bool retain;
};

class FakeTransport : public ITransport {
  public:
    explicit FakeTransport(bool connected = true) noexcept : _connected(connected) {}

    void set_connected(bool v) noexcept { _connected = v; }
    void set_publish_ok(bool v) noexcept { _publish_ok = v; }
    // After the next N successful publishes, every subsequent publish returns
    // false (used to test partial-success handling without losing prior calls).
    void fail_after_n_publishes(std::size_t n) noexcept { _fail_after = n; }

    bool connected() const noexcept override { return _connected; }

    bool publish(std::string_view topic, const uint8_t* data, std::size_t len, uint8_t qos,
                 bool retain) noexcept override {
        if (!_publish_ok)
            return false;
        if (_fail_after.has_value() && _publishes.size() >= *_fail_after)
            return false;
        _publishes.push_back(
            {std::string(topic), std::vector<uint8_t>(data, data + len), qos, retain});
        return true;
    }

    bool subscribe(std::string_view topic, uint8_t qos) noexcept override {
        _subscriptions.push_back(std::string(topic));
        (void)qos;
        return true;
    }

    void loop() noexcept override { ++_loop_calls; }

    const std::vector<PublishRecord>& publishes() const noexcept { return _publishes; }
    const std::vector<std::string>& subscriptions() const noexcept { return _subscriptions; }
    uint32_t loop_calls() const noexcept { return _loop_calls; }

    void clear() noexcept {
        _publishes.clear();
        _subscriptions.clear();
        _loop_calls = 0;
    }

  private:
    bool _connected{true};
    bool _publish_ok{true};
    std::optional<std::size_t> _fail_after{};
    std::vector<PublishRecord> _publishes;
    std::vector<std::string> _subscriptions;
    uint32_t _loop_calls{0};
};

} // namespace test
} // namespace wildlife
