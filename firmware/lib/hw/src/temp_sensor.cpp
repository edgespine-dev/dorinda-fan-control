#include <hw/pins.h>
#include <hw/temp_sensor.h>

#include <DallasTemperature.h>
#include <OneWire.h>

namespace {

OneWire one_wire(hw::pins::DS18B20);
DallasTemperature sensor(&one_wire);

}  // namespace

namespace hw {

bool temp_init()
{
    sensor.begin();
    sensor.setWaitForConversion(true);
    return true;
}

bool temp_read_celsius(float* out_c)
{
    if (out_c == nullptr)
    {
        return false;
    }

    sensor.requestTemperatures();
    const float c = sensor.getTempCByIndex(0);

    if (c == DEVICE_DISCONNECTED_C)
    {
        return false;
    }

    *out_c = c;
    return true;
}

}  // namespace hw
