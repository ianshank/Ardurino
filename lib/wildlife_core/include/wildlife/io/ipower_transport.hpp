#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace wildlife {

enum class WakeCause : uint8_t { PowerOn, PirGpio, Timer, Unknown };

class IPowerManager {
  public:
    virtual ~IPowerManager() = default;
    virtual WakeCause wake_cause() const noexcept = 0;
    // Enters deep sleep; only returns if sleep is unsupported (always-on mode).
    virtual void enter_sleep(uint32_t seconds) noexcept = 0;
    // Battery voltage in mV; 0 if unavailable.
    virtual int32_t battery_mv() noexcept = 0;
};

// Raw transport used by MqttPublisher. Adapters implement with PubSubClient.
class ITransport {
  public:
    virtual ~ITransport() = default;
    virtual bool connected() const noexcept = 0;
    virtual bool publish(std::string_view topic, const uint8_t* data, std::size_t len, uint8_t qos,
                         bool retain) noexcept = 0;
    virtual bool subscribe(std::string_view topic, uint8_t qos) noexcept = 0;
    virtual void loop() noexcept = 0;
};

} // namespace wildlife
