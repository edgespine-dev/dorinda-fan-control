#pragma once

#include <stdint.h>

namespace hw {

// Potentiometer on A0 (B50K, 3V3–GND)
void pot_init();
int16_t pot_read_raw();  // ESP8266 ADC, typically 0–1023

}  // namespace hw
