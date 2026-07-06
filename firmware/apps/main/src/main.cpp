#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>

#include <bsp/log.h>
#include <bsp/wifi.h>
#include <cal/matrix.h>
#include <control/policy.h>
#include <dfc_secrets.h>
#include <dfc_build_info.h>
#include <dfc_wifi/dfc_wifi.h>
#include <drivers/ds18b20.h>
#include <drivers/fan_control.h>
#include <drivers/potentiometer.h>
#include <hw/pins.h>

constexpr uint32_t SERIAL_BAUD = 115200;
constexpr uint32_t CONTROL_INTERVAL_MS = 1000;
constexpr uint32_t TEMP_INTERVAL_MS = 5000;
constexpr uint32_t REPORT_INTERVAL_MS = 1000;
constexpr uint32_t CLIMATE_STALE_MS = 120000;

constexpr int16_t POT_ADC_MIN = 5;
constexpr int16_t POT_ADC_MAX = 1024;
constexpr int POT_DEADBAND_PCT = 4;

constexpr uint32_t FAN_RAMP_INTERVAL_MS = 1000;
constexpr uint8_t FAN_RAMP_STEP_PERCENT = 6;
constexpr uint32_t FAN_PWM_HZ = 18000;

// Quick presets (percent of 12 V fan bus). Silent ≈ 1 V minimum crawl.
constexpr int FAN_BUS_NOMINAL_V = 12;
constexpr int SILENT_PERCENTAGE = (1 * 100 + FAN_BUS_NOMINAL_V / 2) / FAN_BUS_NOMINAL_V;
constexpr int FAN_PRESET_OFF_PERCENT = 0;
constexpr int FAN_PRESET_FULL_PERCENT = 100;

struct RemoteClimate
{
    bool valid = false;
    float temp_c = 0.0f;
    float humidity_pct = 0.0f;
    int co2_ppm = -1;
    uint32_t updated_ms = 0;
};

struct LiveData
{
    int16_t pot_raw = 0;
    int pot_pct = 0;
    int fan_setpoint_pct = 0;
    int fan_current_pct = 0;
    uint16_t pwm = 0;
    float temp_c = 0.0f;
    bool temp_ok = false;
    int est_ma = -1;
};

AsyncWebServer server(80);
LiveData g_live;
RemoteClimate g_climate;
uint32_t last_control_ms = 0;
uint32_t last_temp_ms = 0;
uint32_t last_report_ms = 0;
bool g_fan_pwm_ready = false;

static const char* device_ip()
{
    return dfc_wifi_ip();
}

static void ensure_fan_pwm()
{
    if (g_fan_pwm_ready || !control::boot_armed())
    {
        return;
    }
    drivers::fan_control_force_off(hw::pins::FAN_PWM);
    drivers::fan_control_init(hw::pins::FAN_PWM, FAN_RAMP_INTERVAL_MS, FAN_RAMP_STEP_PERCENT,
                              FAN_PWM_HZ);
    g_fan_pwm_ready = true;
    log_printf(LOG_LEVEL_INFO, "fan PWM started (pot boot-armed) — 18 kHz on D5");
}

static const char* mode_string()
{
    return control::mode() == control::Mode::Remote ? "remote" : "manual";
}

static void poll_control()
{
    g_live.pot_raw = drivers::pot_read_raw();
    g_live.pot_pct = drivers::pot_read_value_percentage();

    ensure_fan_pwm();

    const int out_pct = control::output_percent(g_live.pot_pct);
    if (g_fan_pwm_ready)
    {
        drivers::fan_control_set_new_setpoint(out_pct);
        drivers::fan_control_loop();
        g_live.fan_setpoint_pct = drivers::fan_control_get_setpoint();
        g_live.fan_current_pct = drivers::fan_control_get_current_value();
        g_live.pwm = drivers::fan_control_get_pwm_raw();
    }
    else
    {
        g_live.fan_setpoint_pct = 0;
        g_live.fan_current_pct = 0;
        g_live.pwm = 0;
    }
    g_live.est_ma = g_live.pwm == 0 ? 0 : cal::interpolate_ma(g_live.pwm);
}

static void poll_temp()
{
    g_live.temp_ok = drivers::ds18b20_read_celsius(&g_live.temp_c);
}

static void report_status()
{
    if (!dfc_wifi_ready())
    {
        Serial.println(dfc_wifi_is_sta() ? "WiFi lost" : "AP down");
        return;
    }

    const LiveData s = g_live;
    const char* net = dfc_wifi_is_sta() ? "STA" : "AP";
    const uint8_t clients = dfc_wifi_client_count();

    if (s.temp_ok && s.est_ma >= 0)
    {
        Serial.printf("%s %s ip=%s cli=%u Pot=%d (%d%%) Fan=%d%% PWM=%u Temp=%.1fC Est=%d mA%s\n",
                      net, mode_string(), device_ip(), clients, s.pot_raw, s.pot_pct,
                      s.fan_current_pct, s.pwm, s.temp_c, s.est_ma,
                      control::boot_armed() ? "" : " [BOOT OFF]");
    }
    else if (s.temp_ok)
    {
        Serial.printf("%s %s ip=%s cli=%u Pot=%d (%d%%) Fan=%d%% PWM=%u Temp=%.1fC Est=---%s\n",
                      net, mode_string(), device_ip(), clients, s.pot_raw, s.pot_pct,
                      s.fan_current_pct, s.pwm, s.temp_c, control::boot_armed() ? "" : " [BOOT OFF]");
    }
    else
    {
        Serial.printf("%s %s ip=%s cli=%u Pot=%d (%d%%) Fan=%d%% PWM=%u Temp=---C%s\n", net,
                      mode_string(), device_ip(), clients, s.pot_raw, s.pot_pct, s.fan_current_pct,
                      s.pwm, control::boot_armed() ? "" : " [BOOT OFF]");
    }
}

static void handle_json_body(AsyncWebServerRequest* request, uint8_t* data, size_t len,
                             void (*handler)(JsonDocument& doc, AsyncWebServerRequest* request))
{
    JsonDocument doc;
    if (deserializeJson(doc, data, len))
    {
        request->send(400, "application/json", "{\"error\":\"bad json\"}");
        return;
    }
    handler(doc, request);
}

static void setup_web()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        static const char page[] PROGMEM = R"raw(
<!DOCTYPE html><html><head>
<meta charset=utf-8><meta name=viewport content="width=device-width,initial-scale=1">
<title>Dorinda fan</title>
<style>
body{font-family:system-ui,sans-serif;margin:1.25rem;background:#0f172a;color:#e2e8f0;max-width:28rem}
h1{font-size:1rem;color:#64748b;text-transform:uppercase;margin:0 0 .5rem}
.tag{font-size:.85rem;color:#94a3b8;margin-bottom:1rem}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:.75rem;margin-bottom:1.25rem}
.card{background:#1e293b;border-radius:12px;padding:1rem}
.val{font-size:1.5rem;font-weight:700;color:#38bdf8}
.lbl{font-size:.75rem;color:#64748b;margin-top:.25rem}
label{display:block;font-size:.85rem;color:#94a3b8;margin:.75rem 0 .25rem}
input[type=range]{width:100%}
button{width:100%;padding:.65rem;margin-top:.5rem;border-radius:8px;border:none;
background:#2563eb;color:#fff;font-weight:600;font-size:1rem;cursor:pointer}
button.secondary{background:#334155}
.presets{display:grid;grid-template-columns:1fr 1fr 1fr;gap:.5rem;margin-top:1.25rem}
button.preset{padding:1.1rem .5rem;margin-top:0;font-size:1rem}
button.preset-off{background:#475569}
button.preset-silent{background:#0d9488}
button.preset-full{background:#b45309}
.ver{font-size:.7rem;color:#475569;margin-top:2rem;line-height:1.4}
</style></head><body>
<h1>Dorinda fan</h1>
<p class=tag id=mode>…</p>
<div class=grid>
<div class=card><div class=val id=pot>--</div><div class=lbl>Pot %</div></div>
<div class=card><div class=val id=fan>--</div><div class=lbl>Fan out %</div></div>
<div class=card><div class=val id=temp>--</div><div class=lbl>Bay temp</div></div>
<div class=card><div class=val id=ma>--</div><div class=lbl>Est. current</div></div>
</div>
<label>Remote setpoint <span id=sliderVal>42</span>%</label>
<input type=range id=slider min=0 max=100 value=42>
<button id=apply>Apply remote</button>
<button class=secondary id=manual>Back to pot (manual)</button>
<div class=presets>
<button class="preset preset-off" id=preset-off>OFF</button>
<button class="preset preset-silent" id=preset-silent>Silent</button>
<button class="preset preset-full" id=preset-full>FULL</button>
</div>
<p class=ver id=ver>…</p>
<script>
async function applyPreset(p){
  await fetch('/api/fan/setpoint',{method:'PUT',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({percent:p})});
  refresh();
}
async function refresh(){
  const r=await fetch('/api/status');const j=await r.json();
  document.getElementById('mode').textContent='Mode: '+j.mode+(j.mode==='remote'?' ('+j.remote_percent+'%)':'');
  document.getElementById('pot').textContent=j.pot_percent+'%';
  document.getElementById('fan').textContent=j.fan_current+'%';
  document.getElementById('temp').textContent=j.temp_c!=null?j.temp_c.toFixed(1)+'°C':'---';
  document.getElementById('ma').textContent=j.est_ma!=null?j.est_ma+' mA':'---';
}
document.getElementById('slider').oninput=e=>{
  document.getElementById('sliderVal').textContent=e.target.value;
};
document.getElementById('apply').onclick=async()=>{
  const p=parseInt(document.getElementById('slider').value,10);
  await fetch('/api/fan/setpoint',{method:'PUT',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({percent:p})});
  refresh();
};
document.getElementById('manual').onclick=async()=>{
  await fetch('/api/fan/remote',{method:'DELETE'});
  refresh();
};
fetch('/api/presets').then(r=>r.json()).then(p=>{
  document.getElementById('preset-off').onclick=()=>applyPreset(p.off);
  document.getElementById('preset-silent').onclick=()=>applyPreset(p.silent);
  document.getElementById('preset-full').onclick=()=>applyPreset(p.full);
  document.getElementById('preset-silent').textContent='Silent '+p.silent+'%';
});
fetch('/api/version').then(r=>r.json()).then(v=>{
  document.getElementById('ver').textContent=
    v.version+' · dorinda '+v.git_sha+' · bsp '+v.bsp_git_sha;
});
setInterval(refresh,2000);refresh();
</script></body></html>
)raw";
        request->send(200, "text/html", page);
    });

    server.on("/api/version", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[192];
        snprintf(json, sizeof(json),
                 "{\"product\":\"%s\",\"version\":\"%s\",\"git_sha\":\"%s\","
                 "\"bsp_git_sha\":\"%s\",\"build_utc\":\"%s\"}",
                 DFC_PRODUCT, DFC_APP_VERSION, DFC_GIT_SHA, DFC_BSP_GIT_SHA, DFC_BUILD_UTC);
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/api/presets", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[64];
        snprintf(json, sizeof(json), "{\"off\":%d,\"silent\":%d,\"full\":%d}",
                 FAN_PRESET_OFF_PERCENT, SILENT_PERCENTAGE, FAN_PRESET_FULL_PERCENT);
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[384];
        const LiveData s = g_live;
        const uint32_t now = millis();
        const bool climate_fresh =
            g_climate.valid && (now - g_climate.updated_ms < CLIMATE_STALE_MS);

        if (climate_fresh && s.temp_ok && s.est_ma >= 0 && g_climate.co2_ppm >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":%d,\"temp_c\":%.1f,"
                     "\"remote_percent\":%d,\"climate\":{\"temp_c\":%.1f,\"humidity_pct\":%.1f,"
                     "\"co2_ppm\":%d,\"age_ms\":%u}}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, s.est_ma, s.temp_c, control::remote_percent(), g_climate.temp_c,
                     g_climate.humidity_pct, g_climate.co2_ppm, now - g_climate.updated_ms);
        }
        else if (climate_fresh && s.temp_ok && s.est_ma >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":%d,\"temp_c\":%.1f,"
                     "\"remote_percent\":%d,\"climate\":{\"temp_c\":%.1f,\"humidity_pct\":%.1f,"
                     "\"co2_ppm\":null,\"age_ms\":%u}}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, s.est_ma, s.temp_c, control::remote_percent(), g_climate.temp_c,
                     g_climate.humidity_pct, now - g_climate.updated_ms);
        }
        else if (s.temp_ok && s.est_ma >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":%d,\"temp_c\":%.1f,"
                     "\"remote_percent\":%d,\"climate\":null}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, s.est_ma, s.temp_c, control::remote_percent());
        }
        else if (s.temp_ok)
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":null,\"temp_c\":%.1f,"
                     "\"remote_percent\":%d,\"climate\":null}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, s.temp_c, control::remote_percent());
        }
        else if (s.est_ma >= 0)
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":%d,\"temp_c\":null,"
                     "\"remote_percent\":%d,\"climate\":null}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, s.est_ma, control::remote_percent());
        }
        else
        {
            snprintf(json, sizeof(json),
                     "{\"mode\":\"%s\",\"adc\":%d,\"pot_percent\":%d,\"fan_setpoint\":%d,"
                     "\"fan_current\":%d,\"pwm\":%u,\"est_ma\":null,\"temp_c\":null,"
                     "\"remote_percent\":%d,\"climate\":null}",
                     mode_string(), s.pot_raw, s.pot_pct, s.fan_setpoint_pct, s.fan_current_pct,
                     s.pwm, control::remote_percent());
        }
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/api/fan", HTTP_GET, [](AsyncWebServerRequest* request) {
        static char json[128];
        snprintf(json, sizeof(json),
                 "{\"mode\":\"%s\",\"setpoint\":%d,\"current\":%d,\"remote_percent\":%d}",
                 mode_string(), g_live.fan_setpoint_pct, g_live.fan_current_pct,
                 control::remote_percent());
        AsyncWebServerResponse* response = request->beginResponse(200, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/api/fan/setpoint", HTTP_PUT,
              [](AsyncWebServerRequest* request) {},
              nullptr,
              [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
                  handle_json_body(request, data, len,
                                   [](JsonDocument& doc, AsyncWebServerRequest* req) {
                                       if (!doc["percent"].is<int>())
                                       {
                                           req->send(400, "application/json",
                                                     "{\"error\":\"percent required\"}");
                                           return;
                                       }
                                       const int pct = doc["percent"].as<int>();
                                       control::set_remote(pct, g_live.pot_pct);
                                       req->send(200, "application/json", "{\"ok\":true}");
                                   });
              });

    server.on("/api/fan/remote", HTTP_DELETE, [](AsyncWebServerRequest* request) {
        control::clear_remote();
        request->send(200, "application/json", "{\"ok\":true}");
    });

    server.on("/api/climate", HTTP_POST,
              [](AsyncWebServerRequest* request) {},
              nullptr,
              [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t, size_t) {
                  handle_json_body(request, data, len,
                                   [](JsonDocument& doc, AsyncWebServerRequest* req) {
                                       if (doc["temp_c"].is<float>())
                                       {
                                           g_climate.temp_c = doc["temp_c"].as<float>();
                                           g_climate.valid = true;
                                       }
                                       if (doc["humidity_pct"].is<float>())
                                       {
                                           g_climate.humidity_pct = doc["humidity_pct"].as<float>();
                                           g_climate.valid = true;
                                       }
                                       if (doc["co2_ppm"].is<int>())
                                       {
                                           g_climate.co2_ppm = doc["co2_ppm"].as<int>();
                                           g_climate.valid = true;
                                       }
                                       g_climate.updated_ms = millis();

                                       if (doc["fan_percent"].is<int>())
                                       {
                                           const int pct = doc["fan_percent"].as<int>();
                                           control::set_remote(pct, g_live.pot_pct);
                                       }

                                       req->send(200, "application/json", "{\"ok\":true}");
                                   });
              });

    server.onNotFound([](AsyncWebServerRequest* request) {
        request->send(404, "application/json", "{\"error\":\"not found\"}");
    });

    server.begin();
}

void setup()
{
    drivers::fan_control_force_off(hw::pins::FAN_PWM);

    Serial.begin(SERIAL_BAUD);
    delay(100);
    log_init(SERIAL_BAUD);
    set_log_level(LOG_LEVEL_DEBUG);
    log_printf(LOG_LEVEL_INFO, "dorinda main boot heap=%u", ESP.getFreeHeap());
    log_printf(LOG_LEVEL_INFO, "build %s dorinda@%s bsp@%s (%s)", DFC_APP_VERSION, DFC_GIT_SHA,
               DFC_BSP_GIT_SHA, DFC_BUILD_UTC);

    dfc_wifi_prepare();

    drivers::pot_init(hw::pins::POT, POT_ADC_MIN, POT_ADC_MAX);
    drivers::ds18b20_init(hw::pins::DS18B20);
    control::init(POT_DEADBAND_PCT);
    cal::load_embedded_defaults();

    Serial.println();
    Serial.println("Dorinda fan — main app (REST + pot)  [dfc_wifi boot]");
    Serial.println("Safety: fan OFF until pot moves >4% or remote command");

    if (!dfc_wifi_boot())
    {
        Serial.println("*** WiFi start FAILED — check secrets.env ***");
        return;
    }
    log_printf(LOG_LEVEL_INFO, "heap after AP=%u", ESP.getFreeHeap());

    setup_web();
    log_printf(LOG_LEVEL_INFO, "heap after HTTP=%u", ESP.getFreeHeap());

    Serial.printf("Ready: http://%s/  WiFi '%s'  [pot >4%% or remote to arm fan]\n", device_ip(),
                  DFC_WIFI_SSID);
}

void loop()
{
    yield();

    const uint32_t now = millis();

    if (now - last_control_ms >= CONTROL_INTERVAL_MS)
    {
        last_control_ms = now;
        poll_control();
    }

    if (now - last_temp_ms >= TEMP_INTERVAL_MS)
    {
        last_temp_ms = now;
        poll_temp();
    }

    if (now - last_report_ms >= REPORT_INTERVAL_MS)
    {
        last_report_ms = now;
        report_status();
    }
}
