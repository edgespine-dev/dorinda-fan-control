#pragma once

#include <stdint.h>

// Dorinda WiFi boot — same path for TEST 2, TEST 3, and main.
// Call dfc_wifi_prepare() before LittleFS or other flash-heavy init on ESP8266.

void dfc_wifi_prepare();
bool dfc_wifi_boot();
bool dfc_wifi_is_sta();
bool dfc_wifi_ready();
const char* dfc_wifi_ip();
uint8_t dfc_wifi_client_count();
void dfc_wifi_maintain();  // restart AP if beacons dropped (call from loop)
