# ADR-0013: Promote pot, DS18B20, fan_control to bakerbug `drivers/`

**Status:** Accepted  
**Date:** 2026-07-05  
**Supersedes (partially):** [ADR-0011](0011-local-hw-layer-not-bsp-yet.md) — defer-until-second-product no longer applies to these three

## Context

[ADR-0011](0011-local-hw-layer-not-bsp-yet.md) kept `pot`, `temp_sensor`, and `fan_control` in dorinda `hw/` to avoid upstream churn. bakerbug-firmware already has a **`lib/drivers/`** layer (e.g. SCD41) above BSP for reusable chip and actuator code.

We now want:

- One implementation shared across BakerStack firmware projects
- Per-driver **README** with API, deps, init params, and examples — enough for humans and Cursor to wire drivers without reading `.cpp`
- Dorinda keeps only **board config**: pin map + `setup()` init calls with Dorinda-specific values

## Decision

Add three **full drivers** under `bakerbug-firmware/firmware/lib/drivers/`:

| Module file | API prefix | Responsibility |
|-------------|------------|----------------|
| `drivers/ds18b20.h` | `ds18b20_*` | OneWire temp on any GPIO; blocking read |
| `drivers/potentiometer.h` | `pot_*` | ADC pin + calibrated min/max → 0–100 % |
| `drivers/fan_control.h` | `fan_control_*` | Percent setpoint, ramp, PWM to MOSFET; any GPIO |

**Naming rule:** module file name matches the dominant prefix. Use **`fan_control`**, not `fan_pwm` — the driver models setpoint + ramp behaviour, not raw duty cycles.

Each driver gets:

```
firmware/lib/drivers/
├── include/drivers/<name>.h
├── src/<name>.cpp
└── README.md              ← required: API, deps, example, gotchas
```

**API shape** matches [ADR-0012](0012-hw-component-api.md) (init / set / read / get / loop) in namespace `drivers::`.

**Pin and calibration are init arguments**, not hard-coded:

```cpp
drivers::ds18b20_init(data_pin);
drivers::pot_init(adc_pin, adc_min, adc_max);
drivers::fan_control_init(pwm_pin, ramp_interval_ms, ramp_step_percent, pwm_hz);
drivers::fan_control_set_new_setpoint(percent);
drivers::fan_control_loop();  // every loop()
```

### What stays in dorinda-fan-control

After migration, **no driver implementation** lives in this repo — only wiring:

| In this repo | Example |
|--------------|---------|
| `hw/pins.h` | `POT=A0`, `DS18B20=D2`, `FAN_PWM=D5` |
| `setup()` init calls | see below |
| App logic | override, cal matrix, HTTP, safety |

```cpp
// firmware/tests/test3_pwm_matrix/src/main.cpp (or apps/main)
#include <drivers/ds18b20.h>
#include <drivers/potentiometer.h>
#include <drivers/fan_control.h>
#include <hw/pins.h>

void setup() {
    drivers::pot_init(hw::pins::POT, 5, 1024);
    drivers::ds18b20_init(hw::pins::DS18B20);
    drivers::fan_control_init(hw::pins::FAN_PWM, 200, 6, 18000);
}

void loop() {
    drivers::fan_control_loop();
    const int pct = drivers::pot_read_value_percentage();
    drivers::fan_control_set_new_setpoint(pct);
    // ...
}
```

No long-term `hw/pot.cpp` wrappers — optional only during a short migration PR.

### Driver README template (Cursor-friendly)

Each `README.md` must include:

1. **One-line purpose**
2. **Hardware** — what chip/circuit; external pull-ups, voltage limits
3. **PlatformIO deps** — e.g. OneWire, DallasTemperature
4. **API table** — init, read/set, get, loop
5. **Minimal example** — `setup()` + `loop()` snippet
6. **Gotchas** — blocking time (DS18B20 ~750 ms), never call from async HTTP handlers, ESP8266 PWM freq range
7. **Bench checklist** — how to verify on serial before trusting in app

## Consequences

**Positive:**

- Second BakerStack project copies driver + README, not source from dorinda
- Cursor can discover drivers via bakerbug tree and README without ADR archaeology
- BSP stays platform-only; drivers stay chip/actuator-only

**Negative:**

- Submodule bump workflow when driver APIs change
- One migration PR in dorinda to swap `hw/` implementations for `drivers::` calls

**Follow-ups:**

1. ~~Implement the three drivers + READMEs in bakerbug-firmware~~ — done in submodule `vendor/bakerbug-firmware/firmware/lib/drivers/`
2. ~~Point dorinda `platformio.ini` at submodule `lib/drivers`~~ — done
3. ~~Delete dorinda `hw/*.cpp` driver bodies; keep `hw/pins.h`~~ — done
4. ~~Push bakerbug-firmware submodule commit upstream~~ — done (`26d0ef9`)
