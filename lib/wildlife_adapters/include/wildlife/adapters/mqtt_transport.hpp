#pragma once

#include <Client.h>
#include <PubSubClient.h>

#include <string>
#include <string_view>

#include "wildlife/core/units.hpp"
#include "wildlife/io/ipower_transport.hpp"

namespace wildlife {

class MqttTransport final : public ITransport {
public:
    explicit MqttTransport(Client& client) noexcept : _client(client) {}

    void configure(std::string_view host,
                   uint16_t port,
                   uint16_t keepalive_s,
                   uint16_t max_payload_kb) noexcept;

    bool connect(std::string_view client_id,
                 std::string_view username = {},
                 std::string_view password = {}) noexcept;

    bool connected() const noexcept override { return _client.connected(); }
    bool publish(std::string_view topic,
                 const uint8_t* data,
                 std::size_t len,
                 uint8_t qos,
                 bool retain) noexcept override;
    bool subscribe(std::string_view topic, uint8_t qos) noexcept override;
    void loop() noexcept override { _client.loop(); }

    PubSubClient& client() noexcept { return _client; }

private:
    mutable PubSubClient _client;
    std::string          _host;
    uint16_t             _port{1883};
};

} // namespace wildlife