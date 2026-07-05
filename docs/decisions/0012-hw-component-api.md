# ADR-0012: `hw/` component API — init, set, get, loop

**Status:** Accepted  
**Date:** 2026-07-05

## Context

The local `hw/` library wraps board-specific peripherals (pot, DS18B20, fan PWM). Without a shared convention, test programs and the main app duplicate scaling logic (e.g. ADC min/max → percent) and leak implementation details (raw PWM 0–1023) into control code.

We want:

- One place for calibration and pin wiring
- App code that reads in **domain units** (percent, °C) not chip registers
- A predictable pattern when adding the next sensor or actuator

## Decision

Every hardware component follows a **small, consistent API** in namespace `drivers::` (implemented in bakerbug-firmware; Dorinda supplies pins via `hw::pins`):

| Role | Naming | Purpose |
|------|--------|---------|
| **Init** | `<name>_init(...)` | One-time setup: pins, calibrated ranges, timing parameters |
| **Command** | `<name>_set_*` / `<name>_read_*` | Write a desired value or perform a blocking read |
| **State** | `<name>_get_*` | Return last commanded or derived state in domain units |
| **Peek** | `<name>_get_*_raw` or `<name>_read_raw` | Low-level value for debug, dashboard, or calibration only |
| **Loop** | `<name>_*_loop()` | Non-blocking periodic work — call from `loop()`, never from HTTP callbacks |

### Rules

1. **Pins live in `hw/pins.h`** — components take a pin argument in `_init` only when the same driver might bind to different pins across boards; otherwise use `pins::` constants inside the module.
2. **Domain units at the boundary** — apps use 0–100 % for fan setpoint and pot; raw PWM/ADC stays inside `hw/` (or `_raw` / `_peek` helpers).
3. **No I/O in web handlers** — sensor reads and `fan_control_loop()` run in `loop()`; HTTP serves cached state (see TEST 2 crash fix).
4. **One pair of files per component** — `include/hw/<name>.h` + `src/<name>.cpp`.
5. **BSP vs `drivers/`** — platform-wide reuse (WiFi, TLS) stays in bakerbug BSP; chip/actuator drivers in bakerbug `drivers/`; Dorinda pin map in `hw/pins.h` ([ADR-0013](0013-promote-hw-drivers-to-bakerbug.md)).

### Current components (bakerbug `drivers/`)

| Module | Init | Command / read | Get / peek | Loop |
|--------|------|----------------|------------|------|
| `potentiometer` | `pot_init(pin, min, max)` | `pot_read_value_percentage()` | `pot_read_raw()` | — |
| `ds18b20` | `ds18b20_init(pin)` | `ds18b20_read_celsius(&out)` | — | — |
| `fan_control` | `fan_control_init(pin, ramp_ms, step_pct, hz)` | `fan_control_set_new_setpoint(%)` | `fan_control_get_setpoint()`, `fan_control_get_current_value()`, `fan_control_get_pwm_raw()` | `fan_control_loop()` |

### Adding a new component

1. Add pin(s) to `hw/pins.h` if needed; document in [hardware.md](../hardware.md).
2. Create `hw/<name>.h` and `hw/<name>.cpp` using the table above.
3. Expose only what the app needs — hide `analogWrite`, OneWire, etc.
4. Wire into the smallest TEST that proves the hardware; extend main app only after bench sign-off.
5. If the API is non-obvious, add one line to the component header and link here.

Full policy and layering diagram: [architecture.md — Hardware layer](../architecture.md#hardware-layer-hw).

## Consequences

**Positive:**

- Test mains stay thin: `pot_init(5, 1024)` once, then `pot_read_value_percentage()` everywhere.
- Fan ramp and PWM frequency are encapsulated; control policy only sees percent.
- New contributors know where to put code and how to name functions.

**Negative:**

- Slight indirection vs inline `analogRead` / `analogWrite`.
- `_raw` escape hatches must be used sparingly (calibration matrix still keys on PWM knots).

**Follow-ups:**

- Main app `ControlPolicy` calls `hw::` only — no duplicate scaling.
- Promote a driver to bakerbug BSP when a second repo needs the same abstraction.
