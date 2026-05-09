#include "wildlife/app/app_state_machine.hpp"

#include <unity.h>

using namespace wildlife::app;

void setUp() {}
void tearDown() {}

static void check(AppState from, AppEvent ev, AppState expected_state, AppAction expected_action0) {
    auto r = transition(from, ev);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(expected_state), static_cast<int>(r.new_state));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(expected_action0), static_cast<int>(r.actions[0]));
}

// OTA trigger from Active
void test_ota_check_due_enters_updating() {
    check(AppState::Active, AppEvent::OtaCheckDue, AppState::Updating, AppAction::CheckOta);
}

// OTA available → apply
void test_ota_available_triggers_apply() {
    check(AppState::Updating, AppEvent::OtaAvailable, AppState::Updating, AppAction::ApplyOta);
}

// OTA applied → reboot
void test_ota_applied_triggers_reboot() {
    check(AppState::Updating, AppEvent::OtaApplied, AppState::Updating, AppAction::Reboot);
}

// OTA failed → back to Active
void test_ota_failed_returns_to_active() {
    check(AppState::Updating, AppEvent::OtaFailed, AppState::Active, AppAction::StartInference);
}

// Network drops during OTA
void test_ota_network_down_goes_to_error() {
    check(AppState::Updating, AppEvent::NetworkDown, AppState::Error, AppAction::ScheduleRetry);
}

// No update found — dedicated OtaUpToDate event, not ConfigLoaded
void test_ota_up_to_date_returns_to_active_with_mark_valid() {
    auto r = transition(AppState::Updating, AppEvent::OtaUpToDate);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppState::Active), static_cast<int>(r.new_state));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::MarkValid), static_cast<int>(r.actions[0]));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::StartInference),
                          static_cast<int>(r.actions[1]));
}

// ConfigLoaded in Updating state must now be unhandled (was mistakenly reused)
void test_updating_config_loaded_is_unhandled() {
    auto r = transition(AppState::Updating, AppEvent::ConfigLoaded);
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppState::Updating), static_cast<int>(r.new_state));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::None), static_cast<int>(r.actions[0]));
    TEST_ASSERT_EQUAL_INT(static_cast<int>(AppAction::None), static_cast<int>(r.actions[1]));
}

// Fatal during OTA → reboot
void test_ota_fatal_triggers_reboot() {
    check(AppState::Updating, AppEvent::FatalError, AppState::Error, AppAction::Reboot);
}

// Existing Active transitions still work (regression)
void test_active_sleep_now_regression() {
    check(AppState::Active, AppEvent::SleepNow, AppState::Sleeping, AppAction::EnterSleep);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_ota_check_due_enters_updating);
    RUN_TEST(test_ota_available_triggers_apply);
    RUN_TEST(test_ota_applied_triggers_reboot);
    RUN_TEST(test_ota_failed_returns_to_active);
    RUN_TEST(test_ota_network_down_goes_to_error);
    RUN_TEST(test_ota_up_to_date_returns_to_active_with_mark_valid);
    RUN_TEST(test_updating_config_loaded_is_unhandled);
    RUN_TEST(test_ota_fatal_triggers_reboot);
    RUN_TEST(test_active_sleep_now_regression);
    return UNITY_END();
}