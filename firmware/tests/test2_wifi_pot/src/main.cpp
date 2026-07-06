#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include <bsp/log.h>
#include <dfc_secrets.h>
#include <dfc_wifi/dfc_wifi.h>
#include <drivers/ds18b20.h>
#include <drivers/potentiometer.h>
#include <hw/pins.h>

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t SENSOR_INTERVAL_MS = 1000;

constexpr int16_t POT_ADC_MIN = 5;
constexpr int16_t POT_ADC_MAX = 1024;

struct LiveData
{
    int16_t pot_raw = 0;
    int pot_pct = 0;
    float temp_c = 0.0f;
    bool temp_ok = false;
};

AsyncWebServer server(80);
LiveData g_live;
uint32_t last_sensor_ms = 0;

static void poll_sensors()
{
    g_live.pot_raw = drivers::pot_read_raw();
    g_live.pot_pct = drivers::pot_read_value_percentage();
    g_live.temp_ok = drivers::ds18b20_read_celsius(&g_live.temp_c);
}

static void setup_web()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char page[1024];
        static char temp_buf[16];
        const LiveData s = g_live;
        const char* temp_val = "---";
        if (s.temp_ok)
        {
            snprintf(temp_buf, sizeof(temp_buf), "%.1f°C", s.temp_c);
            temp_val = temp_buf;
        }
        snprintf(page, sizeof(page),
                 "<!DOCTYPE html><html><head>"
                 "<meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\">"
                 "<meta http-equiv=refresh content=2>"
                 "<title>Dorinda</title>"
                 "<style>"
                 "body{font-family:system-ui,sans-serif;margin:1.25rem;background:#0f172a;color:#e2e8f0}"
                 "h1{font-size:1rem;color:#64748b;margin:0 0 1rem;text-transform:uppercase;letter-spacing:.04em}"
                 ".grid{display:grid;grid-template-columns:1fr 1fr;gap:.75rem}"
                 ".card{background:#1e293b;border-radius:12px;padding:1rem}"
                 ".val{font-size:2rem;font-weight:700;color:#38bdf8}"
                 ".lbl{font-size:.75rem;color:#64748b;margin-top:.25rem}"
                 "</style></head><body>"
                 "<h1>Dorinda fan</h1><div class=grid>"
                 "<div class=card><div class=val>%d%%</div><div class=lbl>Pot</div></div>"
                 "<div class=card><div class=val>%d</div><div class=lbl>ADC</div></div>"
                 "<div class=card><div class=val>%s</div><div class=lbl>Temp</div></div>"
                 "</div></body></html>",
                 s.pot_pct, s.pot_raw, temp_val);
        request->send(200, "text/html", page);
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[96];
        const LiveData s = g_live;
        if (s.temp_ok)
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"temp_c\":%.1f}",
                     s.pot_raw, s.pot_pct, s.temp_c);
        }
        else
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"temp_c\":null}",
                     s.pot_raw, s.pot_pct);
        }
        request->send(200, "application/json", json);
    });

    server.begin();
}

void setup()
{
    Serial.begin(SERIAL_BAUD);
    delay(100);
    log_init(SERIAL_BAUD);
    set_log_level(LOG_LEVEL_DEBUG);

    dfc_wifi_prepare();

    drivers::pot_init(hw::pins::POT, POT_ADC_MIN, POT_ADC_MAX);
    drivers::ds18b20_init(hw::pins::DS18B20);
    poll_sensors();

    Serial.println();
    Serial.println("TEST 2 — WiFi + pot + temp (no fan PWM)");

    if (!dfc_wifi_boot())
    {
        Serial.println("WiFi start FAILED — check secrets.env (password >= 8 chars for AP)");
        return;
    }

    setup_web();
}

void loop()
{
    yield();

    const uint32_t now = millis();
    if (now - last_sensor_ms >= SENSOR_INTERVAL_MS)
    {
        last_sensor_ms = now;
        poll_sensors();

        if (!dfc_wifi_ready())
        {
            Serial.println(dfc_wifi_is_sta() ? "WiFi lost" : "AP down");
            return;
        }

        if (dfc_wifi_is_sta())
        {
            if (g_live.temp_ok)
            {
                Serial.printf("STA ip=%s Pot=%d (%d%%) Temp=%.1fC\n",
                              dfc_wifi_ip(), g_live.pot_raw, g_live.pot_pct, g_live.temp_c);
            }
            else
            {
                Serial.printf("STA ip=%s Pot=%d (%d%%) Temp=---C\n",
                              dfc_wifi_ip(), g_live.pot_raw, g_live.pot_pct);
            }
        }
        else if (g_live.temp_ok)
        {
            Serial.printf("AP ip=%s clients=%u Pot=%d (%d%%) Temp=%.1fC\n",
                          dfc_wifi_ip(), dfc_wifi_client_count(),
                          g_live.pot_raw, g_live.pot_pct, g_live.temp_c);
        }
        else
        {
            Serial.printf("AP ip=%s clients=%u Pot=%d (%d%%) Temp=---C\n",
                          dfc_wifi_ip(), dfc_wifi_client_count(),
                          g_live.pot_raw, g_live.pot_pct);
        }
    }
}
