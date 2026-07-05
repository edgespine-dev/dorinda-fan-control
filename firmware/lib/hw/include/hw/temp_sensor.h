#pragma once

#include <stdbool.h>
#include <stdint.h>

namespace hw {

// DS18B20 on D2, 4.7 kΩ pull-up to 3V3
bool temp_init();
bool temp_read_celsius(float* out_c);  // false if sensor missing / disconnected

}  // namespace hw
