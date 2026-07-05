#include <hw/fan_output.h>
#include <hw/pins.h>

#include <Arduino.h>

namespace hw {

namespace {

constexpr uint32_t FAN_PWM_HZ = 20000;
constexpr uint16_t FAN_PWM_MAX = 1023;

uint16_t g_pwm = 0;

}  // namespace

void fan_init()
{
    pinMode(pins::FAN_PWM, OUTPUT);
    analogWriteRange(FAN_PWM_MAX);
    analogWriteFreq(FAN_PWM_HZ);
    fan_set_pwm(0);
}

void fan_set_pwm(uint16_t duty)
{
    if (duty > FAN_PWM_MAX)
    {
        duty = FAN_PWM_MAX;
    }
    g_pwm = duty;
    analogWrite(pins::FAN_PWM, duty);
}

void fan_set_percent(int pct)
{
    if (pct < 0)
    {
        pct = 0;
    }
    if (pct > 100)
    {
        pct = 100;
    }
    fan_set_pwm(static_cast<uint16_t>((pct * FAN_PWM_MAX) / 100));
}

uint16_t fan_get_pwm()
{
    return g_pwm;
}

}  // namespace hw
