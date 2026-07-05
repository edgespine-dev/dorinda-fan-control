# ADR-0008: Thermal safety thresholds in main app V2

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Electronics bay on a boat can overheat. Fan must protect enclosure even if operator sets low speed. Version 1 (TEST 1) deliberately has **no safety logic** to simplify sensor bring-up.

## Decision

Defer safety to **main app V2** (after M1–M9 functional requirements):

| Condition | Action |
|-----------|--------|
| Temp > **70 °C** | Force PWM 100 %, warn on web UI |
| Temp > **85 °C** | **FAULT** state: PWM disabled, latched, alarm on Serial |

Thresholds stored as named constants, not magic numbers.

## Consequences

**Positive:**

- TEST 1 stays minimal
- Safety rules applied in one place (`ControlPolicy` / `FanOutput`)

**Negative:**

- Window between first main app release and V2 without auto safety — acceptable for bench; **do not deploy to boat** until V2 passes

**Follow-ups:**

- Validate 70/85 °C against real bay behaviour; may adjust after sea trials
