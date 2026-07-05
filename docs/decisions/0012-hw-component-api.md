# ADR-0012: Driver component API — init, set, get, loop

**Status:** Accepted  
**Date:** 2026-07-05  
**Related:** [ADR-0013](0013-promote-hw-drivers-to-bakerbug.md) (where drivers live)

## Context

Peripheral code (pot, DS18B20, fan PWM) needs a shared calling convention. Without it, test programs duplicate scaling logic (ADC min/max → percent) and leak implementation details (raw PWM 0–1023) into app code.

We want:

- App code in **domain units** (percent, °C) — not chip registers
- A predictable pattern when adding the next sensor or actuator
- Init parameters (pin, calibration) passed from the product repo, not hard-coded in drivers

## Decision

Every bakerbug **`drivers/`** component follows this API shape in namespace `drivers::`. Dorinda supplies board pins via `hw::pins` and calls `_init()` in `setup()` ([ADR-0013](0013-promote-hw-drivers-to-bakerbug.md)).

| Role | Naming | Purpose |
|------|--------|---------|
| **Init** | `<name>_init(...)` | One-time setup: pin, calibrated ranges, timing parameters |
| **Command** | `<name>_set_*` / `<name>_read_*` | Write a desired value or perform a blocking read |
| **State** | `<name>_get_*` | Return last commanded or derived state in domain units |
| **Peek** | `<name>_get_*_raw` or `<name>_read_raw` | Low-level value for debug, dashboard, or calibration only |
| **Loop** | `<name>_*_loop()` | Non-blocking periodic work — call from `loop()`, never from HTTP callbacks |

### Rules

1. **Pins are init arguments** — product repo defines `hw/pins.h`; drivers never hard-code Dorinda wiring.
2. **Domain units at the boundary** — fan and pot use 0–100 %; temperature in °C. Raw PWM/ADC only via `_raw` helpers.
3. **No I/O in web handlers** — sensor reads and `fan_control_loop()` run in `loop()`; HTTP serves cached state (see TEST 2 crash fix).
4. **One pair of files per driver** — `include/drivers/<name>.h` + `src/<name>.cpp` in bakerbug-firmware.
5. **BSP vs `drivers/`** — WiFi/TLS in BSP; chip/actuator code in `drivers/` ([ADR-0013](0013-promote-hw-drivers-to-bakerbug.md)).

### Current drivers

| Module | Init | Command / read | Get / peek | Loop |
|--------|------|----------------|------------|------|
| `potentiometer` | `pot_init(pin, min, max)` | `pot_read_value_percentage()` | `pot_read_raw()` | — |
| `ds18b20` | `ds18b20_init(pin)` | `ds18b20_read_celsius(&out)` | — | — |
| `fan_control` | `fan_control_init(pin, ramp_ms, step_pct, hz)` | `fan_control_set_new_setpoint(%)` | `fan_control_get_setpoint()`, `fan_control_get_current_value()`, `fan_control_get_pwm_raw()` | `fan_control_loop()` |

**Naming:** module file name matches the API prefix (`fan_control_*`, not `fan_pwm_*`).

### Adding a new driver

1. Add driver under `bakerbug-firmware/firmware/lib/drivers/` with README (see ADR-0013 template).
2. Add pin constant(s) to product `hw/pins.h`; document in [hardware.md](../hardware.md).
3. Call `_init()` from product `setup()` with bench-calibrated values.
4. Prove in the smallest TEST before wiring into main app.

Full layering: [architecture.md — Hardware layer](../architecture.md#hardware-layer-hw).

## Consequences

**Positive:**

- Test mains stay thin: one `pot_init()`, then `pot_read_value_percentage()` everywhere.
- Fan ramp and PWM frequency encapsulated; control policy only sees percent.
- Same API whether code lives in bakerbug or (temporarily) in a product repo.

**Negative:**

- Slight indirection vs inline `analogRead` / `analogWrite`.
- `_raw` escape hatches for calibration matrix (PWM knots) must stay rare.

**Follow-ups:**

- Main app `ControlPolicy` calls `drivers::` only — no duplicate scaling.
- New drivers follow this ADR; placement in bakerbug per ADR-0013.
