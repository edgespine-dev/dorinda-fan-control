#include <dfc_wifi/dfc_wifi.h>

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <bsp/log.h>
#include <bsp/wifi.h>
#include <dfc_secrets.h>

namespace {

enum class Mode : uint8_t
{
    Ap,
    Sta,
};

Mode g_mode = Mode::Ap;
String g_ip;
bool g_started = false;

}  // namespace

void dfc_wifi_prepare()
{
    WiFi.persistent(false);
    WiFi.setAutoConnect(false);
    WiFi.setAutoReconnect(false);
    WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

bool dfc_wifi_boot()
{
    dfc_wifi_prepare();

    if (strcmp(DFC_WIFI_MODE, "STA") == 0)
    {
        g_mode = Mode::Sta;
        if (!bsp_wifi_connect(DFC_WIFI_SSID, DFC_WIFI_PASSWORD))
        {
            g_started = false;
            return false;
        }
        g_ip = bsp_wifi_ip();
        g_started = true;
        log_printf(LOG_LEVEL_INFO, "dfc_wifi STA OK ip=%s", g_ip.c_str());
        bsp_wifi_print_diag();
        return true;
    }

    g_mode = Mode::Ap;
    if (!bsp_wifi_softap_begin(DFC_WIFI_SSID, DFC_WIFI_PASSWORD))
    {
        g_started = false;
        return false;
    }
    g_ip = bsp_wifi_softap_ip();
    g_started = true;
    log_printf(LOG_LEVEL_INFO, "dfc_wifi AP OK ip=%s clients=%u", g_ip.c_str(),
               bsp_wifi_softap_station_count());
    bsp_wifi_print_diag();
    return true;
}

bool dfc_wifi_is_sta()
{
    return g_mode == Mode::Sta;
}

bool dfc_wifi_ready()
{
    if (!g_started)
    {
        return false;
    }
    if (dfc_wifi_is_sta())
    {
        return bsp_wifi_is_connected();
    }
    return WiFi.getMode() == WIFI_AP && WiFi.softAPSSID().length() > 0 &&
           WiFi.softAPIP() != IPAddress(0, 0, 0, 0);
}

const char* dfc_wifi_ip()
{
    return g_ip.c_str();
}

uint8_t dfc_wifi_client_count()
{
    if (!g_started || dfc_wifi_is_sta())
    {
        return 0;
    }
    return bsp_wifi_softap_station_count();
}

void dfc_wifi_maintain()
{
    if (!g_started || g_mode == Mode::Sta)
    {
        return;
    }
    if (dfc_wifi_ready())
    {
        return;
    }
    log_printf(LOG_LEVEL_WARN, "AP lost — restarting ssid='%s' heap=%u", DFC_WIFI_SSID,
               ESP.getFreeHeap());
    if (bsp_wifi_softap_begin(DFC_WIFI_SSID, DFC_WIFI_PASSWORD))
    {
        g_ip = bsp_wifi_softap_ip();
        log_printf(LOG_LEVEL_INFO, "AP restarted ip=%s clients=%u", g_ip.c_str(),
                   bsp_wifi_softap_station_count());
    }
}
