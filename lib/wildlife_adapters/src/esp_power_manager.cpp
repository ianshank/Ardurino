#include "wildlife/adapters/esp_power_manager.hpp"

#include <Arduino.h>
#include <driver/gpio.h>
#include <esp_sleep.h>

namespace wildlife {

WakeCause EspPowerManager::wake_cause() const noexcept {
    switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0:
    case ESP_SLEEP_WAKEUP_EXT1:
        return WakeCause::PirGpio;
    case ESP_SLEEP_WAKEUP_TIMER:
        return WakeCause::Timer;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
        return WakeCause::PowerOn;
    default:
        return WakeCause::Unknown;
    }
}

void EspPowerManager::enter_sleep(uint32_t seconds) noexcept {
    if (_power_cfg.mode_enum == PowerMode::AlwaysOn) {
        return;
    }

    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(_board->pir_pin),
                                 _power_cfg.wake_level ? 1 : 0);
    if (seconds > 0U) {
        esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(seconds) * 1000000ULL);
    }
    esp_deep_sleep_start();
}

int32_t EspPowerManager::battery_mv() noexcept {
    if (!_board->has_battery_adc) {
        return 0;
    }

    pinMode(_board->battery_adc_pin, INPUT);
    const uint8_t sample_count = _battery_cfg.samples == 0 ? 1 : _battery_cfg.samples;

    int64_t sum_mv = 0;
    for (uint8_t index = 0; index < sample_count; ++index) {
        sum_mv += analogReadMilliVolts(_board->battery_adc_pin);
    }

    const double averaged = static_cast<double>(sum_mv) / static_cast<double>(sample_count);
    const double scaled = averaged * _battery_cfg.divider_ratio;
    return scaled <= 0.0 ? 0 : static_cast<int32_t>(scaled + 0.5);
}

} // namespace wildlife