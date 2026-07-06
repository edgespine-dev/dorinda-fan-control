#pragma once

#include <stdint.h>

namespace control {

enum class Mode : uint8_t
{
    Manual,  // pot authority
    Remote,  // REST / phone / forepeak node — until pot moves
};

void init(int pot_deadband_pct = 4);

// Fan stays at 0 % until pot moves > deadband from boot position, or remote commands arm.
bool boot_armed();
void arm_boot();

// Call each loop with current pot %; returns fan setpoint 0–100.
int output_percent(int pot_pct);

Mode mode();
int remote_percent();

void set_remote(int percent_0_100, int pot_pct_at_lock);
void clear_remote();

}  // namespace control
