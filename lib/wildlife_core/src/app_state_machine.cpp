#include "wildlife/app/app_state_machine.hpp"

namespace wildlife {
namespace app {

using S = AppState;
using E = AppEvent;
using A = AppAction;

static constexpr AppAction kNone = AppAction::None;

TransitionResult transition(AppState state, AppEvent event) noexcept {
    // Unhandled guard — callers detect when both actions are None AND new_state
    // equals the original state (unchanged identity).
    const TransitionResult unhandled{state, {kNone, kNone}};

    switch (state) {

    case S::Boot:
        switch (event) {
            case E::WakeUp:       return {S::ConfigLoad, {A::LoadConfig,   kNone}};
            case E::ConfigFailed: return {S::SafeMode,   {A::SetSafeMode,  A::OpenPortal}};
            default: break;
        }
        break;

    case S::ConfigLoad:
        switch (event) {
            case E::ConfigLoaded: return {S::Network,  {A::ClearSafeMode,  A::ConnectNetwork}};
            case E::ConfigFailed: return {S::SafeMode, {A::SetSafeMode,    A::OpenPortal}};
            default: break;
        }
        break;

    case S::SafeMode:
        switch (event) {
            case E::ProvisionDone: return {S::ConfigLoad, {A::LoadConfig,   kNone}};
            case E::FatalError:    return {S::Error,      {A::Reboot,       kNone}};
            default: break;
        }
        break;

    case S::Network:
        switch (event) {
            case E::NetworkUp:   return {S::Active,   {A::StartInference,  kNone}};
            case E::NetworkDown: return {S::Error,    {A::ScheduleRetry,   kNone}};
            case E::FatalError:  return {S::Error,    {A::Reboot,          kNone}};
            default: break;
        }
        break;

    case S::Active:
        switch (event) {
            case E::SleepNow:     return {S::Sleeping,  {A::EnterSleep,     kNone}};
            case E::NetworkDown:  return {S::Error,     {A::ScheduleRetry,  kNone}};
            case E::FatalError:   return {S::Error,     {A::Reboot,         kNone}};
            case E::OtaCheckDue:  return {S::Updating,  {A::CheckOta,       kNone}};
            default: break;
        }
        break;

    case S::Sleeping:
        switch (event) {
            case E::WakeUp:      return {S::ConfigLoad, {A::LoadConfig,    kNone}};
            default: break;
        }
        break;

    case S::Error:
        switch (event) {
            case E::RetryTimeout:  return {S::Network,  {A::ConnectNetwork, kNone}};
            case E::NetworkUp:     return {S::Active,   {A::StartInference, kNone}};
            case E::FatalError:    return {S::Error,    {A::Reboot,         kNone}};
            default: break;
        }
        break;

    case S::Updating:
        switch (event) {
            case E::OtaAvailable:  return {S::Updating,    {A::ApplyOta,       kNone}};
            case E::OtaApplied:    return {S::Updating,    {A::Reboot,         kNone}};
            case E::OtaFailed:     return {S::Active,      {A::StartInference, kNone}};
            case E::NetworkDown:   return {S::Error,       {A::ScheduleRetry,  kNone}};
            case E::FatalError:    return {S::Error,       {A::Reboot,         kNone}};
            // No update found — back to normal operation
            case E::OtaUpToDate:   return {S::Active,      {A::MarkValid,      A::StartInference}};
            default: break;
        }
        break;

    default:
        break;
    }

    return unhandled;
}

} // namespace app
} // namespace wildlife
