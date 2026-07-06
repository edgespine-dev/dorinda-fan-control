# ADR-0015: Fan PWM off at power-on until pot is moved

**Status:** Accepted  
**Date:** 2026-07-06

## Context

The fan drives a **12 V bay load** via MOSFET on D5. On power-up, a drifting pot wiper, noise, or stale PWM state could spin the fan unexpectedly — unacceptable in a confined electronics enclosure.

## Decision

1. **`fan_control_force_off(D5)`** is the first action in `setup()` — pin driven LOW before any driver init.
2. **`fan_control_init()` is deferred** until the operator moves the pot **more than 4 %** from the position read at first control tick (`control::boot_armed()`).
3. Until armed, `output_percent()` returns **0**; REST setpoints are accepted but PWM stays off.

Deadband matches pot override logic (ADR-0007): **4 %** of span (`POT_DEADBAND_PCT`).

## Consequences

**Positive:**

- No fan spin on plug-in regardless of pot position or remote API calls
- Explicit human acknowledgement before 12 V load

**Negative:**

- Phone / forepeak presets have no effect until pot is nudged once per power cycle
- Document in UI and operator README

**Follow-ups:**

- Optional “armed” indicator on web dashboard (serial already prints `[BOOT OFF]`)
