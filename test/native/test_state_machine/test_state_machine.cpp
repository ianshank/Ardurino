#include "wildlife/app/app_state_machine.hpp"

#include <unity.h>

using namespace wildlife::app;

void setUp() {}
void tearDown() {}

// Checks that a transition produces the expected new state and first action.
static void check(AppState from, AppEvent ev, AppState expected_state, AppAction expected_action0) {
    auto r = transition(from, ev);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(expected_state), static_cast<int>(r.new_state));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(expected_action0), static_cast<int>(r.actions[0]));
}

// An unhandled event returns the same state with no actions.
static void check_unhandled(AppState state, AppEvent ev) {
    auto r = transition(state, ev);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(state), static_cast<int>(r.new_state));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::None), static_cast<int>(r.actions[0]));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::None), static_cast<int>(r.actions[1]));
}

void test_boot_wake_up() {
    check(AppState::Boot, AppEvent::WakeUp, AppState::ConfigLoad, AppAction::LoadConfig);
}

void test_config_load_success() {
    check(AppState::ConfigLoad, AppEvent::ConfigLoaded, AppState::Network,
          AppAction::ClearSafeMode);
}

void test_config_load_failure() {
    check(AppState::ConfigLoad, AppEvent::ConfigFailed, AppState::SafeMode, AppAction::SetSafeMode);
}

void test_safe_mode_provision_done() {
    check(AppState::SafeMode, AppEvent::ProvisionDone, AppState::ConfigLoad, AppAction::LoadConfig);
}

void test_network_up() {
    check(AppState::Network, AppEvent::NetworkUp, AppState::Active, AppAction::StartInference);
}

void test_network_down_from_network() {
    check(AppState::Network, AppEvent::NetworkDown, AppState::Error, AppAction::ScheduleRetry);
}

void test_active_sleep_now() {
    check(AppState::Active, AppEvent::SleepNow, AppState::Sleeping, AppAction::EnterSleep);
}

void test_active_network_down() {
    check(AppState::Active, AppEvent::NetworkDown, AppState::Error, AppAction::ScheduleRetry);
}

void test_sleeping_wake_up() {
    check(AppState::Sleeping, AppEvent::WakeUp, AppState::ConfigLoad, AppAction::LoadConfig);
}

void test_error_retry_timeout() {
    check(AppState::Error, AppEvent::RetryTimeout, AppState::Network, AppAction::ConnectNetwork);
}

void test_error_network_up_skips_to_active() {
    check(AppState::Error, AppEvent::NetworkUp, AppState::Active, AppAction::StartInference);
}

void test_fatal_error_triggers_reboot() {
    check(AppState::Active, AppEvent::FatalError, AppState::Error, AppAction::Reboot);
}

void test_boot_config_failed() {
    check(AppState::Boot, AppEvent::ConfigFailed, AppState::SafeMode, AppAction::SetSafeMode);
    // Second action should be OpenPortal
    auto r = transition(AppState::Boot, AppEvent::ConfigFailed);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::OpenPortal), static_cast<int>(r.actions[1]));
}

void test_unhandled_event_returns_same_state() {
    check_unhandled(AppState::Active, AppEvent::ProvisionDone);
    check_unhandled(AppState::Sleeping, AppEvent::FatalError);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_boot_wake_up);
    RUN_TEST(test_config_load_success);
    RUN_TEST(test_config_load_failure);
    RUN_TEST(test_safe_mode_provision_done);
    RUN_TEST(test_network_up);
    RUN_TEST(test_network_down_from_network);
    RUN_TEST(test_active_sleep_now);
    RUN_TEST(test_active_network_down);
    RUN_TEST(test_sleeping_wake_up);
    RUN_TEST(test_error_retry_timeout);
    RUN_TEST(test_error_network_up_skips_to_active);
    RUN_TEST(test_fatal_error_triggers_reboot);
    RUN_TEST(test_boot_config_failed);
    RUN_TEST(test_unhandled_event_returns_same_state);
    return UNITY_END();
}
