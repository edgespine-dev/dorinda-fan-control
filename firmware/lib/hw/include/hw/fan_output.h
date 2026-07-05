#pragma once

#include <stdint.h>

namespace hw {

// D5 → MOSFET TRIG. 20 kHz / 10-bit — above audible range, within module 0–20 kHz spec.
void fan_init();
void fan_set_pwm(uint16_t duty_0_1023);
void fan_set_percent(int percent_0_100);
uint16_t fan_get_pwm();

}  // namespace hw
