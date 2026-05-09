#pragma once

// Wildlife Core — IProvisioner
// Abstracts over a captive-portal provisioner (e.g., WiFiManager).
// Decouples AppStateMachine action OpenPortal from any Arduino library.

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace wildlife {

// Filled by the provisioner after a successful portal session.
struct ProvisionResult {
    std::string ssid;
    std::string password;
    // Arbitrary key-value pairs from custom portal fields.
    // Keys match RuntimeConfig field names so the config subsystem can
    // apply them generically without the provisioner knowing field semantics.
    std::vector<std::pair<std::string, std::string>> fields;
};

class IProvisioner {
  public:
    virtual ~IProvisioner() = default;

    // Start the captive portal.  ap_name / ap_password come from RuntimeConfig.
    // Returns false immediately if the portal cannot start.
    virtual bool start(std::string_view ap_name, std::string_view ap_password) noexcept = 0;

    // Non-blocking poll; returns true once credentials are ready.
    virtual bool poll() noexcept = 0;

    // True when start() + poll() completed successfully.
    virtual bool credentials_ready() const noexcept = 0;

    // Retrieve the provisioned values. Valid only when credentials_ready().
    virtual ProvisionResult result() const noexcept = 0;
};

} // namespace wildlife
