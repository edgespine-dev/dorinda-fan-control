# ADR-0015: Fan PWM off at power-on until pot is moved

**Status:** Accepted  
**Date:** 2026-07-06

## Context

The fan drives a **12 V bay load** via MOSFET on D5. On power-up, a drifting pot wiper, noise, or stale PWM state could spin the fan unexpectedly — unacceptable in a confined electronics enclosure.

## Decision

1. **`fan_control_force_off(D5)`** is the first action in `setup()` — pin driven LOW before any driver init.
2. **`fan_control_init()` is deferred** until boot is **armed** by either:
   - moving the pot **more than 4 %** from the first control tick, or
   - any **remote** command (`PUT /api/fan/setpoint`, presets, `POST /api/climate` with `fan_percent`)
3. Until armed, `output_percent()` returns **0** for manual pot-only path.

Deadband matches pot override logic (ADR-0007): **4 %** of span (`POT_DEADBAND_PCT`).

## Consequences

**Positive:**

- No fan spin on plug-in regardless of pot position or remote API calls
- Explicit human acknowledgement before 12 V load

**Negative:**

- Operator must deliberately arm (pot nudge or remote) before 12 V load — no accidental spin on plug-in

**Follow-ups:**

- Optional “armed” indicator on web dashboard (serial already prints `[BOOT OFF]`)
