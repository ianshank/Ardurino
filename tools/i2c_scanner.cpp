// I2C Scanner — Debug utility for XIAO ESP32S3 + Grove Vision AI V2
//
// Scans the I2C bus on GPIO5 (SDA) / GPIO6 (SCL) every 3 seconds.
// Expected output: "Found device at 0x62" when the Grove Vision AI V2
// is physically connected and powered.
//
// Usage:
//   pio run --environment xiao_esp32s3_sense_dev \
//       --target upload \
//       -DWILDLIFE_I2C_SCANNER=1
//
// Or simply flash this file temporarily via Arduino IDE.

#if defined(WILDLIFE_I2C_SCANNER)

#include <Arduino.h>
#include <Wire.h>

static constexpr int SDA_PIN = 5;
static constexpr int SCL_PIN = 6;

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000) { /* wait for USB CDC */ }

    Wire.begin(SDA_PIN, SCL_PIN);
    Serial.println();
    Serial.println("==============================");
    Serial.println("  I2C Scanner — XIAO ESP32S3");
    Serial.printf("  SDA=GPIO%d  SCL=GPIO%d\n", SDA_PIN, SCL_PIN);
    Serial.println("==============================");
    Serial.println();
}

void loop() {
    Serial.println("Scanning I2C bus...");
    int devices = 0;

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("  ✓ Found device at 0x%02X", addr);
            if (addr == 0x62) {
                Serial.print("  ← Grove Vision AI V2");
            }
            Serial.println();
            devices++;
        }
    }

    if (devices == 0) {
        Serial.println("  ✗ No I2C devices found! Check wiring:");
        Serial.println("    - Is the XIAO stacked on the Grove Vision AI V2?");
        Serial.println("    - Are USB-C connectors facing the same direction?");
        Serial.println("    - Is the Grove cable firmly seated?");
    } else {
        Serial.printf("  %d device(s) found.\n", devices);
    }

    Serial.println("---");
    delay(3000);
}

#endif // WILDLIFE_I2C_SCANNER
