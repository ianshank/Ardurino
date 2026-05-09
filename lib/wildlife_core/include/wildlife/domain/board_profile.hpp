#pragma once

// Wildlife Core — Compile-time, immutable board wiring profile.
// Pins and hardware constants live here, NOT in RuntimeConfig.
// A corrupt config.json can never alter GPIO assignments.

#include <cstdint>

namespace wildlife {

struct BoardProfile {
    const char* board_id;

    // Grove I²C (Grove Vision AI V2)
    uint8_t  i2c_sda;
    uint8_t  i2c_scl;
    uint32_t i2c_freq_hz;
    uint8_t  sscma_i2c_addr;   // default 0x62

    // Wake / UI
    uint8_t  pir_pin;
    uint8_t  boot_btn_pin;     // long-press opens captive portal
    uint32_t portal_hold_ms;

    // Battery ADC
    uint8_t  battery_adc_pin;
    bool     has_battery_adc;

    // PSRAM
    bool     psram_required;

    // On-board camera (OV2640 on Sense variant)
    bool     has_esp32_camera;

    // Camera pin assignments (only meaningful when has_esp32_camera == true)
    struct CameraPins {
        int8_t d0{-1},  d1{-1},  d2{-1},  d3{-1};
        int8_t d4{-1},  d5{-1},  d6{-1},  d7{-1};
        int8_t xclk{-1};
        int8_t pclk{-1};
        int8_t vsync{-1};
        int8_t href{-1};
        int8_t sda{-1};   // SCCB / camera I²C SDA
        int8_t scl{-1};   // SCCB / camera I²C SCL
        int8_t reset{-1};
        int8_t pwdn{-1};
    } camera_pins;
};

} // namespace wildlife
