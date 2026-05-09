#pragma once

// Wildlife Core — Board profile for XIAO ESP32S3 Sense.
// Compile-time constant: selected via -D WILDLIFE_BOARD=xiao_esp32s3_sense.

#include "wildlife/domain/board_profile.hpp"

namespace wildlife {
namespace boards {

// Grove connector: SDA=GPIO5, SCL=GPIO6 (standard Seeed XIAO Grove pinout).
// PIR: GPIO2 (breadboard default; change via external circuit, not config).
// BOOT btn: GPIO0.  Battery ADC: GPIO1 (via 1:2 divider on Sense carrier).
inline constexpr BoardProfile kXiaoEsp32S3Sense{
    .board_id = "xiao_esp32s3_sense",
    .i2c_sda = 5,
    .i2c_scl = 6,
    .i2c_freq_hz = 400000,
    .sscma_i2c_addr = 0x62,
    .pir_pin = 2,
    .boot_btn_pin = 0,
    .portal_hold_ms = 3000,
    .battery_adc_pin = 1,
    .has_battery_adc = true,
    .psram_required = true,
    .has_esp32_camera = true,
};

} // namespace boards
} // namespace wildlife
