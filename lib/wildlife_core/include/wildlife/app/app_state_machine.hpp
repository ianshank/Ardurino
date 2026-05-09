#pragma once

// Wildlife Core — Application state machine.
// Pure function: no I/O, no allocations on hot path, deterministic.

#include <array>
#include <cstdint>

namespace wildlife {
namespace app {

enum class AppState : uint8_t {
    Boot = 0,   // Initial power-on state
    ConfigLoad, // Config loading in progress
    SafeMode,   // Config failed; captive portal forced
    Network,    // Connecting Wi-Fi + MQTT
    Active,     // Inference + publish loop
    Sleeping,   // Entered / entering deep sleep
    Error,      // Recoverable connection error; waiting to retry
    Updating,   // OTA in progress
    _COUNT
};

enum class AppEvent : uint8_t {
    ConfigLoaded = 0, // Config parsed successfully
    ConfigFailed,     // Config parse / migrate failed
    ProvisionDone,    // Captive portal provisioning complete
    NetworkUp,        // Wi-Fi + MQTT both connected
    NetworkDown,      // Connection dropped (transient)
    SleepNow,         // active_seconds elapsed
    WakeUp,           // Deep sleep timer or PIR wake
    RetryTimeout,     // Backoff delay expired; try reconnect
    FatalError,       // Unrecoverable; must reboot
    OtaCheckDue,      // Periodic OTA check timer fired
    OtaAvailable,     // IOtaService::check() returned Available
    OtaApplied,       // Firmware written; ready to reboot
    OtaFailed,        // OTA write error; stay running
    OtaUpToDate,      // OTA check completed; firmware is current
    _COUNT
};

enum class AppAction : uint8_t {
    None = 0,
    LoadConfig,     // Begin reading config from store
    OpenPortal,     // Start captive portal provisioner
    ConnectNetwork, // Initiate Wi-Fi + MQTT connection
    StartInference, // Begin Grove Vision AI polling loop
    EnterSleep,     // Call IPowerManager::enter_sleep()
    ScheduleRetry,  // Arm retry timer via RetryPolicy
    Reboot,         // Trigger MCU reset
    SetSafeMode,    // Set safe_mode flag in status payload
    ClearSafeMode,  // Clear safe_mode flag
    CheckOta,       // Call IOtaService::check()
    ApplyOta,       // Call IOtaService::apply() then reboot
    MarkValid,      // Call IOtaService::mark_valid() after stable boot
};

struct TransitionResult {
    AppState new_state;
    std::array<AppAction, 2> actions; // second slot is None when unused
};

// Pure state-transition function.
// Returns {current_state, {None, None}} for unhandled (state, event) pairs
// so callers can detect and log them without crashing.
TransitionResult transition(AppState state, AppEvent event) noexcept;

} // namespace app
} // namespace wildlife
