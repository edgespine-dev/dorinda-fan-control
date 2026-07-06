#include <control/policy.h>

#include <Arduino.h>

namespace control {

namespace {

Mode g_mode = Mode::Manual;
int g_remote_pct = 0;
int g_pot_lock_pct = 0;
int g_deadband_pct = 4;
bool g_boot_armed = false;
int g_boot_pot_pct = -1;

bool try_boot_arm(int pot_pct)
{
    if (g_boot_armed)
    {
        return true;
    }
    if (g_boot_pot_pct < 0)
    {
        g_boot_pot_pct = pot_pct;
        return false;
    }
    const int delta = pot_pct - g_boot_pot_pct;
    if (delta < 0 ? -delta : delta <= g_deadband_pct)
    {
        return false;
    }
    g_boot_armed = true;
    Serial.printf("control: boot armed (pot %d%%, idle at %d%%)\n", pot_pct, g_boot_pot_pct);
    return true;
}

}  // namespace

void init(int pot_deadband_pct)
{
    g_mode = Mode::Manual;
    g_remote_pct = 0;
    g_pot_lock_pct = 0;
    g_deadband_pct = pot_deadband_pct > 0 ? pot_deadband_pct : 1;
    g_boot_armed = false;
    g_boot_pot_pct = -1;
}

bool boot_armed()
{
    return g_boot_armed;
}

void arm_boot()
{
    if (g_boot_armed)
    {
        return;
    }
    g_boot_armed = true;
    Serial.println("control: boot armed (remote)");
}

int output_percent(int pot_pct)
{
    if (!try_boot_arm(pot_pct))
    {
        return 0;
    }

    if (g_mode == Mode::Remote)
    {
        const int delta = pot_pct - g_pot_lock_pct;
        if (delta < 0 ? -delta : delta > g_deadband_pct)
        {
            g_mode = Mode::Manual;
            Serial.printf("control: pot moved (%d -> %d) — remote cleared\n", g_pot_lock_pct, pot_pct);
            return pot_pct;
        }
        return g_remote_pct;
    }
    return pot_pct;
}

Mode mode()
{
    return g_mode;
}

int remote_percent()
{
    return g_remote_pct;
}

void set_remote(int percent_0_100, int pot_pct_at_lock)
{
    if (percent_0_100 < 0)
    {
        percent_0_100 = 0;
    }
    if (percent_0_100 > 100)
    {
        percent_0_100 = 100;
    }
    arm_boot();
    g_mode = Mode::Remote;
    g_remote_pct = percent_0_100;
    g_pot_lock_pct = pot_pct_at_lock;
}

void clear_remote()
{
    g_mode = Mode::Manual;
}

}  // namespace control
