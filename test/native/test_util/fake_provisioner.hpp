#pragma once

// Test utility — FakeProvisioner: controllable IProvisioner.

#include "wildlife/io/iprovisioner.hpp"

namespace wildlife {
namespace test {

class FakeProvisioner : public IProvisioner {
  public:
    void set_start_result(bool ok) noexcept { _start_ok = ok; }
    void set_poll_result(bool done) noexcept { _poll_done = done; }
    void set_result(ProvisionResult r) noexcept { _result = std::move(r); }

    bool start(std::string_view ap_name, std::string_view ap_password) noexcept override {
        _started_ap_name = std::string(ap_name);
        _started_ap_password = std::string(ap_password);
        ++_start_count;
        return _start_ok;
    }

    bool poll() noexcept override {
        ++_poll_count;
        return _poll_done;
    }

    bool credentials_ready() const noexcept override { return _poll_done; }

    ProvisionResult result() const noexcept override { return _result; }

    uint32_t start_count() const noexcept { return _start_count; }
    uint32_t poll_count() const noexcept { return _poll_count; }
    const std::string& started_ap() const noexcept { return _started_ap_name; }

  private:
    bool _start_ok{true};
    bool _poll_done{false};
    ProvisionResult _result;
    std::string _started_ap_name;
    std::string _started_ap_password;
    uint32_t _start_count{0};
    uint32_t _poll_count{0};
};

} // namespace test
} // namespace wildlife
