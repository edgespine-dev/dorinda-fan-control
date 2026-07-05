#include <hw/pins.h>
#include <hw/pot.h>

#include <Arduino.h>

namespace hw {

void pot_init()
{
    // A0 is the only ESP8266 ADC pin; no pinMode required.
}

int16_t pot_read_raw()
{
    return static_cast<int16_t>(analogRead(pins::POT));
}

}  // namespace hw
