#include <Arduino.h>

#include <hw/pot.h>
#include <hw/temp_sensor.h>

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t REPORT_INTERVAL_MS = 1000;

uint32_t last_report_ms = 0;

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(100);

    hw::pot_init();
    hw::temp_init();

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

    const int pot = hw::pot_read_raw();

    float temp_c = 0.0f;
    if (hw::temp_read_celsius(&temp_c))
    {
        Serial.printf("Pot=%d Temp=%.1fC\n", pot, temp_c);
    }
    else
    {
        Serial.printf("Pot=%d Temp=---C\n", pot);
    }
}
