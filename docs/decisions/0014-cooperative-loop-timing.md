# ADR-0014: Cooperative loop timing — not a real-time system

**Status:** Accepted  
**Date:** 2026-07-06

## Context

The fan controller shares one ESP8266 CPU between WiFi (AP + HTTP), OneWire (DS18B20), ADC (pot), and 18 kHz PWM. Early main-app bring-up showed **AP “OK” in serial but invisible on phones** when sensor polling and HTTP setup competed with the radio stack.

This is a **boat accessory**, not a motion controller. Pot position, fan ramp, and temperature do not need sub-100 ms response.

## Decision

Run product logic on **fixed, slow ticks**. The main loop only calls `yield()` between ticks so the WiFi stack and AsyncTCP get CPU.

| Task | Interval | Notes |
|------|----------|-------|
| WiFi / HTTP | continuous | `yield()` every `loop()` |
| Pot, control policy, fan ramp step | **1 s** | `CONTROL_INTERVAL_MS` |
| DS18B20 temperature | **5 s** | `TEMP_INTERVAL_MS` — OneWire blocks ~750 ms |
| Serial status line | **1 s** | `REPORT_INTERVAL_MS` |
| Fan ramp | **1 s / 6 %** | `FAN_RAMP_INTERVAL_MS` aligned with control tick |

**Boot order:** WiFi AP → HTTP `server.begin()` → sensors. No OneWire reads before the radio is up.

## Consequences

**Positive:**

- AP stable on phone and laptop (validated after adopting this model)
- Simple mental model for operators and future contributors
- Forepeak HTTP client can poll `/api/status` at 1–2 s without mismatch

**Negative:**

- Pot-to-fan response feels up to ~1 s (acceptable on a bay fan)
- Temperature display can lag up to 5 s

**Follow-ups:**

- Safety V2 (ADR-0008) may use a faster temp poll when near 70 °C — only if needed
