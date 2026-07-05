#include <Arduino.h>

#include <drivers/ds18b20.h>
#include <drivers/potentiometer.h>
#include <hw/pins.h>

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t REPORT_INTERVAL_MS = 1000;

// Calibrated in TEST 1 — see docs/test-log.md
constexpr int16_t POT_ADC_MIN = 5;
constexpr int16_t POT_ADC_MAX = 1024;

uint32_t last_report_ms = 0;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(100);

    drivers::pot_init(hw::pins::POT, POT_ADC_MIN, POT_ADC_MAX);
    drivers::ds18b20_init(hw::pins::DS18B20);

    Serial.println();
    Serial.println("TEST 1 — pot + DS18B20 (no WiFi, no fan PWM)");
}

void loop()
{
    const uint32_t now = millis();
    if (now - last_report_ms < REPORT_INTERVAL_MS)
    {
        return;
    }
    last_report_ms = now;

    const int pot = drivers::pot_read_raw();
    const int pct = drivers::pot_read_value_percentage();

    float temp_c = 0.0f;
    if (drivers::ds18b20_read_celsius(&temp_c))
    {
        Serial.printf("Pot=%d (%d%%) Temp=%.1fC\n", pot, pct, temp_c);
    }
    else
    {
        Serial.printf("Pot=%d (%d%%) Temp=---C\n", pot, pct);
    }
}
