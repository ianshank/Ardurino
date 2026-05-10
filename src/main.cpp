// Wildlife Spotter — composition root.
//
// Phase 1 stub replaced with a SSCMA streaming smoke test:
//   * Initialises Wire, SscmaI2cSource (Grove Vision AI V2 over I²C),
//     and SscmaJpegGrabber.
//   * Each poll_interval_ms, runs invoke(), prints detection count and
//     the first detection (if any), then attempts to grab the JPEG and
//     prints the byte count.
//   * Pure serial output — no WiFi, no MQTT. Use this to verify the I²C
//     link to the Grove Vision AI V2 and that the loaded model encodes
//     JPEG previews. Wiring for AppRuntime / Publisher / MQTT lands later.
//
// Greppable log lines (prefix "WSPOT|"):
//   WSPOT|boot       fw=<ver>
//   WSPOT|i2c.begin  sda=<n> scl=<n> hz=<n>
//   WSPOT|sscma.begin ok|fail
//   WSPOT|invoke     ok=<0|1> healthy=<0|1> dets=<n> ms=<n>
//   WSPOT|det        idx=<n> class=<id> score=<n> x=<n> y=<n> w=<n> h=<n>
//   WSPOT|jpeg       bytes=<n>      (or bytes=0 when preview is disabled)

#include <Arduino.h>
#include <Wire.h>

#ifndef WILDLIFE_FW_VERSION
#define WILDLIFE_FW_VERSION "unknown_ide_build"
#endif

#include "wildlife/adapters/esp_clock.hpp"
#include "wildlife/adapters/serial_logger.hpp"
#include "wildlife/adapters/sscma_i2c_source.hpp"
#include "wildlife/adapters/sscma_jpeg_grabber.hpp"
#include "wildlife/boards/xiao_esp32s3_sense.hpp"
#include "wildlife/domain/runtime_config.hpp"

namespace {

// Use static storage so destructors don't fire on reset and so we don't
// rely on heap allocation order between setup() and loop().
wildlife::SerialLogger g_logger(Serial, wildlife::LogLevel::Debug);
wildlife::EspClock g_clock;
wildlife::RuntimeConfig::Inference g_inference_cfg{}; // defaults are fine for smoke test
wildlife::SscmaI2cSource* g_source = nullptr;
wildlife::SscmaJpegGrabber* g_grabber = nullptr;

} // namespace

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 2000) { /* wait for USB CDC */
    }

    Serial.println();
    Serial.print("WSPOT|boot       fw=");
    Serial.println(WILDLIFE_FW_VERSION);

    const auto& profile = wildlife::boards::kXiaoEsp32S3Sense;

    Serial.print("WSPOT|i2c.begin  sda=");
    Serial.print(profile.i2c_sda);
    Serial.print(" scl=");
    Serial.print(profile.i2c_scl);
    Serial.print(" hz=");
    Serial.println(profile.i2c_freq_hz);

    // SscmaI2cSource::begin() will call Wire.begin() with the profile pins,
    // so we don't double-init Wire here.

    // The Wild Bird Detection model may not support JPEG encoding or causes timeouts.
    // Disable it for the smoke test to verify basic detection works first.
    g_inference_cfg.sscma_encode = 0;

    static wildlife::SscmaI2cSource source(Wire, profile, g_inference_cfg, &g_clock, &g_logger);
    g_source = &source;

    const bool ok = g_source->begin();
    Serial.print("WSPOT|sscma.begin ");
    Serial.println(ok ? "ok" : "fail");

    if (ok) {
        Serial.print("WSPOT|sscma.info ");
        Serial.println(g_source->sscma().info().c_str());
    }

    static wildlife::SscmaJpegGrabber grabber(g_source->sscma(), &g_logger);
    g_grabber = &grabber;
}

void loop() {
    if (!g_source) {
        delay(1000);
        return;
    }

    const uint32_t t0 = millis();
    const bool ok = g_source->invoke();
    const uint32_t dt = millis() - t0;

    const auto& dets = g_source->detections();

    Serial.print("WSPOT|invoke     ok=");
    Serial.print(ok ? 1 : 0);
    Serial.print(" healthy=");
    Serial.print(g_source->healthy() ? 1 : 0);
    Serial.print(" dets=");
    Serial.print(static_cast<unsigned int>(dets.size()));
    Serial.print(" ms=");
    Serial.println(dt);

    for (size_t i = 0; i < dets.size(); ++i) {
        const auto& d = dets[i];
        Serial.print("WSPOT|det        idx=");
        Serial.print(static_cast<unsigned int>(i));
        Serial.print(" class=");
        Serial.print(d.class_id);
        Serial.print(" score=");
        Serial.print(d.score);
        Serial.print(" x=");
        Serial.print(d.bbox.x);
        Serial.print(" y=");
        Serial.print(d.bbox.y);
        Serial.print(" w=");
        Serial.print(d.bbox.w);
        Serial.print(" h=");
        Serial.println(d.bbox.h);
    }

    if (g_grabber) {
        wildlife::JpegBuffer buf;
        const bool got = g_grabber->grab(buf);
        Serial.print("WSPOT|jpeg       bytes=");
        Serial.println(got ? static_cast<unsigned int>(buf.data.size()) : 0u);
    }

    // Honour the configured poll cadence (default 500 ms).
    delay(g_inference_cfg.poll_interval_ms);
}
