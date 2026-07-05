#pragma once

#include <Arduino.h>
#include <stdint.h>

// Wemos D1 Mini pin map — Dorinda enclosure fan controller
namespace hw::pins {

constexpr uint8_t POT = A0;
constexpr uint8_t DS18B20 = D2;
constexpr uint8_t FAN_PWM = D5;  // TEST 3+ / main app — not used in TEST 1

}  // namespace hw::pins
