# ADR-0011: Local `hw/` layer; defer DS18B20/MOSFET to BSP

**Status:** Accepted  
**Date:** 2026-07-05

## Context

bakerbug-firmware BSP covers WiFi, GPIO, TLS, etc. This project adds **DS18B20** (OneWire) and **MOSFET PWM** (single `analogWrite` pin). Options:

| Approach | Fit |
|----------|-----|
| Add `bsp/ds18b20` + `bsp/pwm` to bakerbug now | Reusable if second product uses same parts |
| Inline in `main.cpp` | Fast but unreadable as app grows |
| **Project-local `hw/` library** | Clear names, KISS, no upstream churn for one-off pins |

MOSFET control is one GPIO/PWM output. Pot is one ADC read. Neither needs a full BSP driver until a second repo shares the same abstraction.

## Decision

- Add thin **`firmware/lib/hw/`** with `pot`, `temp_sensor`, and `pins.h`
- **TEST 1** uses `hw/` only — no bakerbug BSP linked yet
- **TEST 2+** pulls in `vendor/bakerbug-firmware` BSP for WiFi
- Promote DS18B20 to bakerbug BSP **only when** a second project needs it

## Consequences

**Positive:**

- Main and tests call `hw::` with a consistent API — see [ADR-0012](0012-hw-component-api.md)
- Pin map lives in one file (`hw/pins.h`)
- bakerbug stays focused on platform, not every sensor board

**Negative:**

- DS18B20 code duplicated if another BakerStack product uses same sensor (acceptable until then)

**Follow-ups:**

- ~~Promote DS18B20 to bakerbug BSP~~ → **bakerbug `drivers/`** per [ADR-0013](0013-promote-hw-drivers-to-bakerbug.md) (`ds18b20`, `potentiometer`, `fan_control` + READMEs)
- `hw/fan_control` reference impl — **ported** to `drivers/fan_control` in bakerbug submodule (2026-07-05)
