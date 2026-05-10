#include <Arduino.h>
#include <Seeed_Arduino_SSCMA.h>
#include <WiFi.h>

#include <cstring>

#include "app_httpd.h"

#ifndef CAMERA_WIFI_SSID
#define CAMERA_WIFI_SSID ""
#endif

#ifndef CAMERA_WIFI_PASSWORD
#define CAMERA_WIFI_PASSWORD ""
#endif

#ifndef CAMERA_FALLBACK_AP_SSID
#define CAMERA_FALLBACK_AP_SSID "WSPOT-Camera"
#endif

// CAMERA_FALLBACK_AP_PASS controls the Wi-Fi password of the soft-AP started
// when STA join fails. The build deliberately ships NO default value:
//   * Production builds MUST inject a strong, per-deployment value via
//     tools/camera_web_env.py (env var CAMERA_FALLBACK_AP_PASS).
//   * Dev builds may opt-in to an open AP by defining
//     CAMERA_FALLBACK_AP_OPEN=1 (no password). This is logged loudly.
// Anything else is a hard compile error so we cannot silently ship a
// well-known credential.
#if !defined(CAMERA_FALLBACK_AP_PASS) && !defined(CAMERA_FALLBACK_AP_OPEN)
#error "Define CAMERA_FALLBACK_AP_PASS (>=8 chars) at build time, or set CAMERA_FALLBACK_AP_OPEN=1 for an explicitly open dev AP."
#endif
#if defined(CAMERA_FALLBACK_AP_OPEN) && !defined(CAMERA_FALLBACK_AP_PASS)
#define CAMERA_FALLBACK_AP_PASS ""
#endif

// SSCMA AI instance is defined in app_httpd.cpp
extern SSCMA AI;

namespace {

constexpr char     kFallbackApSsid[]       = CAMERA_FALLBACK_AP_SSID;
constexpr char     kFallbackApPassword[]   = CAMERA_FALLBACK_AP_PASS;
constexpr uint32_t kWifiConnectTimeoutMs   = 20000;

// ---------------------------------------------------------------------------
// Grove Vision AI V2 initialization handshake
// ---------------------------------------------------------------------------
// On boot the Grove module does NOT auto-select a model or enable its sensor.
// Without those steps, AT+INVOKE returns code 4 (CMD_EIO) with zeroed model
// fields and the camera stream produces no JPEG frames. This helper drains
// boot output, dumps state for diagnostics, then explicitly selects model 1
// and enables sensor 1 so the web UI's INVOKE command succeeds.

void grove_log_response(const char* resp, size_t len) {
    if (len == 0) {
        return;
    }
    Serial.print("[Grove] ");
    Serial.write(reinterpret_cast<const uint8_t*>(resp), len);
    if (resp[len - 1] != '\n') {
        Serial.println();
    }
}

void grove_send_at(const char* body) {
    Serial.print("[XIAO->Grove] AT+");
    Serial.println(body);
    AI.write("AT+", 3);
    AI.write(body, std::strlen(body));
    AI.write("\r\n", 2);
}

void grove_pump(uint32_t ms) {
    const uint32_t start = millis();
    while (millis() - start < ms) {
        AI.fetch(grove_log_response);
        delay(5);
    }
}

void init_grove_ai() {
    Serial.println("[Grove] draining boot output...");
    grove_pump(1500);

    Serial.println("[Grove] querying device state...");
    grove_send_at("ID?");      grove_pump(300);
    grove_send_at("VER?");     grove_pump(300);
    grove_send_at("STAT?");    grove_pump(300);
    grove_send_at("MODELS?");  grove_pump(800);
    grove_send_at("MODEL?");   grove_pump(300);
    grove_send_at("SENSORS?"); grove_pump(500);
    grove_send_at("ALGOS?");   grove_pump(500);
    grove_send_at("ALGO?");    grove_pump(300);

    // SENSOR command syntax is AT+SENSOR=<id>,<enabled>,<opt_id>
    // From SENSORS? probe: sensor id=1, opt_id=2 means "640x480 Auto"
    Serial.println("[Grove] selecting model 1, sensor 1 @ 640x480, default algorithm...");
    grove_send_at("MODEL=1");      grove_pump(1500);
    grove_send_at("SENSOR=1,1,2"); grove_pump(1500);

    Serial.println("[Grove] re-checking state after init...");
    grove_send_at("MODEL?");  grove_pump(300);
    grove_send_at("SENSOR?"); grove_pump(300);
    grove_send_at("ALGO?");   grove_pump(300);

    // NOTE: do NOT auto-issue AT+SAMPLE=-1 here. On this Grove firmware build
    // SAMPLE=-1 triggers a hard reset (observed: ESP-ROM banner re-emits over
    // I2C), which clears sensor state and leaves Grove in a degraded mode
    // where SENSOR=1,1,2 is accepted but immediately auto-disabled (SENSOR?
    // returns state:0). The stock web UI at http://<device>/ is the correct
    // way to start a stream: open it in a browser and click the Start button,
    // which triggers SAMPLE/INVOKE through the JS controller as the firmware
    // expects.
    //
    // Direct GET of /stream/frame or /stream/result will hang because the
    // queue is only fed when SAMPLE/INVOKE are running, AND because Grove's
    // sensor pipeline self-disables whenever no model binary is bound. To
    // unblock real streaming, flash a .tflite onto the Grove via the
    // SenseCraft AI mobile app or the SenseCraft Web Tool at
    // https://wiki.seeedstudio.com/grove_vision_ai_v2/

    Serial.println("[Grove] init handshake complete.");
    Serial.println("[Grove] To stream, open http://<device-ip>/ in a browser");
    Serial.println("       and click Start. Direct GET of /stream/frame will");
    Serial.println("       hang until Grove has a .tflite model loaded.");
    Serial.println("       Push a model with the SenseCraft AI app or web tool.");
}

void print_stream_urls(IPAddress ip) {
    Serial.print("Camera Ready: http://");
    Serial.println(ip);
    Serial.print("Frame Stream: http://");
    Serial.print(ip);
    Serial.println(":8080/stream/frame");
    Serial.print("Result Stream: http://");
    Serial.print(ip);
    Serial.println(":8080/stream/result");
}

bool connect_station() {
    if (strlen(CAMERA_WIFI_SSID) == 0) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(CAMERA_WIFI_SSID, CAMERA_WIFI_PASSWORD);

    Serial.print("Connecting to WiFi SSID: ");
    Serial.println(CAMERA_WIFI_SSID);

    const uint32_t started_ms = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - started_ms < kWifiConnectTimeoutMs) {
        delay(500);
        Serial.print('.');
    }
    Serial.println();

    return WiFi.status() == WL_CONNECTED;
}

void start_fallback_ap() {
    WiFi.mode(WIFI_AP);
    WiFi.setSleep(false);
    WiFi.softAP(kFallbackApSsid, kFallbackApPassword);

    Serial.print("Fallback AP SSID: ");
    Serial.println(kFallbackApSsid);
#if defined(CAMERA_FALLBACK_AP_OPEN)
    Serial.println("Fallback AP password: <OPEN — dev build, no auth>");
#else
    // Never log the AP password to the serial console — operators can read
    // it from the build configuration / provisioning channel.
    Serial.println("Fallback AP password: <redacted>");
#endif
}

} // namespace

void setup() {
    initSharedBuffer();
    initStatInfo();

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    while (!Serial && millis() < 2000) {}
    Serial.println();
    Serial.println("WSPOT camera_web_server boot");

#if defined(WILDLIFE_FAKE_JPEG)
    Serial.println("[fake-jpeg] WILDLIFE_FAKE_JPEG enabled");
    Serial.println("[fake-jpeg] Grove hardware init is bypassed; /stream/frame");
    Serial.println("            will emit synthetic MJPEG frames for smoke tests.");
#else
    Serial.println("[Grove] Real streaming requires a model-loaded Grove Vision AI V2.");
    Serial.println("        For direct MJPEG smoke without Grove hardware, build");
    Serial.println("        xiao_esp32s3_camera_web_fake instead.");
#endif

    if (connect_station()) {
        Serial.println("WiFi connected");
        print_stream_urls(WiFi.localIP());
    } else {
        Serial.println("WiFi station unavailable; starting fallback AP");
        start_fallback_ap();
        print_stream_urls(WiFi.softAPIP());
    }

#if !defined(WILDLIFE_FAKE_JPEG)
    startRemoteProxy(PROTO_I2C);
    init_grove_ai();
#endif
    startCameraServer();
}

void loop() {
#if !defined(WILDLIFE_FAKE_JPEG)
    loopRemoteProxy();
#endif
    delay(5);
}
