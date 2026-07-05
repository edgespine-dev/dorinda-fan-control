#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include <bsp/log.h>
#include <bsp/wifi.h>
#include <cal/matrix.h>
#include <dfc_secrets.h>
#include <hw/fan_output.h>
#include <hw/pot.h>
#include <hw/temp_sensor.h>

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t REPORT_INTERVAL_MS = 1000;

constexpr int16_t POT_ADC_MIN = 5;
constexpr int16_t POT_ADC_MAX = 1024;

enum class WifiMode { Ap, Sta };

struct LiveData
{
    int16_t pot_raw = 0;
    int pot_pct = 0;
    uint16_t pwm = 0;
    float temp_c = 0.0f;
    bool temp_ok = false;
    int est_ma = -1;
};

AsyncWebServer server(80);
LiveData g_live;
uint32_t last_sensor_ms = 0;
WifiMode g_wifi_mode = WifiMode::Ap;
String g_device_ip;

bool g_cal_hold = false;
uint16_t g_cal_hold_pwm = 0;

static int pot_percent(int16_t raw)
{
    const int span = POT_ADC_MAX - POT_ADC_MIN;
    if (span <= 0)
    {
        return 0;
    }
    int pct = ((static_cast<int>(raw) - POT_ADC_MIN) * 100) / span;
    if (pct < 0)
    {
        return 0;
    }
    if (pct > 100)
    {
        return 100;
    }
    return pct;
}

static uint16_t percent_to_pwm(int pct)
{
    if (pct < 0)
    {
        pct = 0;
    }
    if (pct > 100)
    {
        pct = 100;
    }
    return static_cast<uint16_t>((pct * 1023) / 100);
}

static bool wifi_is_sta()
{
    return g_wifi_mode == WifiMode::Sta;
}

static const char* device_ip()
{
    return g_device_ip.c_str();
}

static bool wifi_start()
{
    if (strcmp(DFC_WIFI_MODE, "STA") == 0)
    {
        g_wifi_mode = WifiMode::Sta;
        if (!bsp_wifi_connect(DFC_WIFI_SSID, DFC_WIFI_PASSWORD))
        {
            return false;
        }
        g_device_ip = bsp_wifi_ip();
        Serial.printf("STA OK ip=%s\n", device_ip());
        return true;
    }

    g_wifi_mode = WifiMode::Ap;
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(DFC_WIFI_SSID, DFC_WIFI_PASSWORD))
    {
        return false;
    }
    g_device_ip = WiFi.softAPIP().toString();
    Serial.printf("AP OK ssid=%s ip=%s\n", DFC_WIFI_SSID, device_ip());
    return true;
}

static bool wifi_ready()
{
    if (wifi_is_sta())
    {
        return bsp_wifi_is_connected();
    }
    return WiFi.getMode() == WIFI_AP;
}

static void apply_fan_output(int pot_pct)
{
    const uint16_t pwm = g_cal_hold ? g_cal_hold_pwm : percent_to_pwm(pot_pct);
    hw::fan_set_pwm(pwm);
}

static void poll_sensors_fast()
{
    g_live.pot_raw = hw::pot_read_raw();
    g_live.pot_pct = pot_percent(g_live.pot_raw);
    apply_fan_output(g_live.pot_pct);
    g_live.pwm = hw::fan_get_pwm();
    g_live.est_ma = cal::interpolate_ma(g_live.pwm);
}

static void poll_sensors_slow()
{
    g_live.temp_ok = hw::temp_read_celsius(&g_live.temp_c);
}

static void setup_web()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char page[1024];
        static char temp_buf[16];
        static char ma_buf[16];
        const LiveData s = g_live;
        const char* temp_val = "---";
        const char* ma_val = "---";
        if (s.temp_ok)
        {
            snprintf(temp_buf, sizeof(temp_buf), "%.1f°C", s.temp_c);
            temp_val = temp_buf;
        }
        if (s.est_ma >= 0)
        {
            snprintf(ma_buf, sizeof(ma_buf), "%d mA", s.est_ma);
            ma_val = ma_buf;
        }
        snprintf(page, sizeof(page),
                 "<!DOCTYPE html><html><head>"
                 "<meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\">"
                 "<meta http-equiv=refresh content=2><title>Dorinda</title>"
                 "<style>"
                 "body{font-family:system-ui,sans-serif;margin:1.25rem;background:#0f172a;color:#e2e8f0}"
                 "h1{font-size:1rem;color:#64748b;margin:0 0 .5rem;text-transform:uppercase}"
                 "nav{margin-bottom:1rem}a{color:#38bdf8;text-decoration:none}"
                 ".grid{display:grid;grid-template-columns:1fr 1fr;gap:.75rem}"
                 ".card{background:#1e293b;border-radius:12px;padding:1rem}"
                 ".val{font-size:1.75rem;font-weight:700;color:#38bdf8}"
                 ".lbl{font-size:.75rem;color:#64748b;margin-top:.25rem}"
                 ".tag{font-size:.8rem;color:#94a3b8;margin-bottom:.75rem}"
                 "</style></head><body>"
                 "<h1>Dorinda fan</h1><nav><a href=/cal>Calibration</a></nav>"
                 "<p class=tag>%s</p><div class=grid>"
                 "<div class=card><div class=val>%d%%</div><div class=lbl>Pot</div></div>"
                 "<div class=card><div class=val>%u</div><div class=lbl>PWM out</div></div>"
                 "<div class=card><div class=val>%s</div><div class=lbl>Temp</div></div>"
                 "<div class=card><div class=val>%s</div><div class=lbl>Est. current</div></div>"
                 "</div></body></html>",
                 g_cal_hold ? "CAL hold" : "Pot → PWM",
                 s.pot_pct, s.pwm, temp_val, ma_val);
        request->send(200, "text/html", page);
    });

    server.on("/cal", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = F(
            "<!DOCTYPE html><html><head>"
            "<meta charset=utf-8><meta name=viewport content=\"width=device-width,initial-scale=1\">"
            "<title>Cal — Dorinda</title>"
            "<style>"
            "body{font-family:system-ui,sans-serif;margin:1.25rem;background:#0f172a;color:#e2e8f0;max-width:28rem}"
            "h1{font-size:1rem;color:#64748b;text-transform:uppercase}"
            "label{display:block;margin:.75rem 0 .25rem;font-size:.85rem;color:#94a3b8}"
            "input,button{width:100%;padding:.6rem;border-radius:8px;border:1px solid #334155;background:#1e293b;color:#e2e8f0;font-size:1rem;box-sizing:border-box}"
            "button{margin-top:1rem;background:#2563eb;border:none;font-weight:600;cursor:pointer}"
            "table{width:100%;border-collapse:collapse;margin-top:1.5rem;font-size:.85rem}"
            "td,th{padding:.4rem;text-align:left;border-bottom:1px solid #334155}"
            "a{color:#38bdf8}"
            "</style></head><body>"
            "<p><a href=/>&larr; Dashboard</a></p>"
            "<h1>PWM &rarr; current matrix</h1>"
            "<form method=POST action=/api/matrix>"
            "<label>PWM duty (0–1023)</label>"
            "<input name=pwm type=number min=0 max=1023 value=512 required>"
            "<label>Measured current (mA)</label>"
            "<input name=current_ma type=number min=0 required>"
            "<label><input type=checkbox name=hold value=1 checked> Hold this PWM (ignore pot while calibrating)</label>"
            "<button type=submit>Save row</button>"
            "</form>"
            "<p style=font-size:.8rem;color:#64748b;margin-top:1rem>"
            "Suggested knots: 0, 64, 128, 192, 256, 384, 512, 768, 1023. Linear interpolation between points.</p>"
            "<table><tr><th>PWM</th><th>mA</th></tr>");

        for (size_t i = 0; i < cal::count(); ++i)
        {
            const cal::Row& r = cal::rows()[i];
            html += F("<tr><td>");
            html += r.pwm;
            html += F("</td><td>");
            html += r.current_ma;
            html += F("</td></tr>");
        }
        html += F("</table></body></html>");
        request->send(200, "text/html", html);
    });

    server.on("/api/matrix", HTTP_POST, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("pwm", true) || !request->hasParam("current_ma", true))
        {
            request->send(400, "text/plain", "missing pwm or current_ma");
            return;
        }

        const uint16_t pwm = static_cast<uint16_t>(request->getParam("pwm", true)->value().toInt());
        const uint16_t ma = static_cast<uint16_t>(request->getParam("current_ma", true)->value().toInt());

        if (!cal::upsert(pwm, ma))
        {
            request->send(507, "text/plain", "matrix full");
            return;
        }
        cal::dump_serial();

        if (request->hasParam("hold", true))
        {
            g_cal_hold = true;
            g_cal_hold_pwm = pwm;
            hw::fan_set_pwm(pwm);
        }

        request->redirect("/cal");
    });

    server.on("/api/hold", HTTP_POST, [](AsyncWebServerRequest* request) {}, nullptr,
              [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
                  JsonDocument doc;
                  if (deserializeJson(doc, data, len))
                  {
                      request->send(400, "text/plain", "bad json");
                      return;
                  }
                  g_cal_hold = doc["hold"] | false;
                  if (g_cal_hold)
                  {
                      g_cal_hold_pwm = doc["pwm"] | 0;
                      hw::fan_set_pwm(g_cal_hold_pwm);
                  }
                  request->send(200, "application/json", "{\"ok\":true}");
              });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[128];
        const LiveData s = g_live;
        if (s.temp_ok && s.est_ma >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"pwm\":%u,\"cal_hold\":%s,\"temp_c\":%.1f,\"est_ma\":%d}",
                     s.pot_raw, s.pot_pct, s.pwm, g_cal_hold ? "true" : "false", s.temp_c, s.est_ma);
        }
        else if (s.temp_ok)
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"pwm\":%u,\"cal_hold\":%s,\"temp_c\":%.1f,\"est_ma\":null}",
                     s.pot_raw, s.pot_pct, s.pwm, g_cal_hold ? "true" : "false", s.temp_c);
        }
        else if (s.est_ma >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"pwm\":%u,\"cal_hold\":%s,\"temp_c\":null,\"est_ma\":%d}",
                     s.pot_raw, s.pot_pct, s.pwm, g_cal_hold ? "true" : "false", s.est_ma);
        }
        else
        {
            snprintf(json, sizeof(json),
                     "{\"adc\":%d,\"percent\":%d,\"pwm\":%u,\"cal_hold\":%s,\"temp_c\":null,\"est_ma\":null}",
                     s.pot_raw, s.pot_pct, s.pwm, g_cal_hold ? "true" : "false");
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

    hw::pot_init();
    hw::temp_init();
    hw::fan_init();
    poll_sensors_fast();
    poll_sensors_slow();

    Serial.println();
    Serial.println("TEST 3 — pot PWM + cal matrix (20 kHz on D5)");

    if (!cal::init())
    {
        Serial.println("cal_matrix init failed");
    }
    else if (cal::count() > 0)
    {
        Serial.println("Loaded cal matrix:");
        cal::dump_serial();
    }

    if (!wifi_start())
    {
        Serial.println("WiFi start FAILED — check secrets.env");
        return;
    }

    setup_web();
}

void loop()
{
    yield();
    poll_sensors_fast();

    const uint32_t now = millis();
    if (now - last_sensor_ms < REPORT_INTERVAL_MS)
    {
        return;
    }
    last_sensor_ms = now;
    poll_sensors_slow();

    if (!wifi_ready())
    {
        return;
    }

    const LiveData s = g_live;
    if (s.temp_ok && s.est_ma >= 0)
    {
        Serial.printf("Pot=%d (%d%%) PWM=%u Temp=%.1fC Est=%d mA%s\n",
                      s.pot_raw, s.pot_pct, s.pwm, s.temp_c, s.est_ma,
                      g_cal_hold ? " [CAL]" : "");
    }
    else if (s.temp_ok)
    {
        Serial.printf("Pot=%d (%d%%) PWM=%u Temp=%.1fC Est=---%s\n",
                      s.pot_raw, s.pot_pct, s.pwm, s.temp_c, g_cal_hold ? " [CAL]" : "");
    }
    else
    {
        Serial.printf("Pot=%d (%d%%) PWM=%u Temp=---C%s\n",
                      s.pot_raw, s.pot_pct, s.pwm, g_cal_hold ? " [CAL]" : "");
    }
}
