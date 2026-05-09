#include "wildlife/adapters/mqtt_transport.hpp"

#include <limits>

namespace wildlife {

void MqttTransport::configure(std::string_view host,
                              uint16_t port,
                              uint16_t keepalive_s,
                              uint16_t max_payload_kb) noexcept {
    _host.assign(host.begin(), host.end());
    _port = port;
    _client.setServer(_host.c_str(), _port);
    _client.setKeepAlive(keepalive_s);

    const std::size_t requested_bytes = static_cast<std::size_t>(max_payload_kb) * wildlife::kBytesPerKb;
    const auto clamped = requested_bytes > std::numeric_limits<uint16_t>::max()
        ? std::numeric_limits<uint16_t>::max()
        : static_cast<uint16_t>(requested_bytes);
    _client.setBufferSize(clamped);
}

bool MqttTransport::connect(std::string_view client_id,
                            std::string_view username,
                            std::string_view password) noexcept {
    const std::string client_id_buf(client_id.begin(), client_id.end());
    const std::string username_buf(username.begin(), username.end());
    const std::string password_buf(password.begin(), password.end());

    if (username_buf.empty()) {
        return _client.connect(client_id_buf.c_str());
    }
    return _client.connect(client_id_buf.c_str(),
                           username_buf.c_str(),
                           password_buf.c_str());
}

bool MqttTransport::publish(std::string_view topic,
                            const uint8_t* data,
                            std::size_t len,
                            uint8_t qos,
                            bool retain) noexcept {
    (void)qos;
    const std::string topic_buf(topic.begin(), topic.end());
    return _client.publish(topic_buf.c_str(), data, static_cast<unsigned int>(len), retain);
}

bool MqttTransport::subscribe(std::string_view topic, uint8_t qos) noexcept {
    const std::string topic_buf(topic.begin(), topic.end());
    return _client.subscribe(topic_buf.c_str(), qos);
}

} // namespace wildlife