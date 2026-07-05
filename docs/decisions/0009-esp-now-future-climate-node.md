# ADR-0009: Future ESP-NOW remote climate node (separate device/repo)

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Long-term goal: **auto mode** — run fan as slowly as possible while keeping enclosure climate acceptable on Dorinda. Temperature inside the **electronics bay** (DS18B20) is not the same as ambient cabin conditions. A remote **humidity + temperature** sensor elsewhere on the boat provides better control input.

WiFi TCP between two ESPs adds router dependency for sensor link. **ESP-NOW** gives direct peer frames without involving the AP for data plane.

## Decision

- Plan a **separate ESP-based climate node** (future repo, e.g. `dorinda-climate-node`)
- Link via **ESP-NOW** to this fan controller
- Stub `ControlMode::Auto` in fan firmware from day one; V1 behaves as Manual/Override only
- **Pot movement always wins** over auto mode (same priority as override drop)

Fan controller keeps WiFi for phone browser; ESP-NOW is additive.

## Consequences

**Positive:**

- Architecture ready for auto mode without rewriting control core
- Sensor placement flexible (cabin vs bilge vs bay)

**Negative:**

- Pairing/MAC management needed later
- Two firmware artifacts to maintain

**Follow-ups:**

- Define ESP-NOW payload struct when climate node work starts
- Confirm ESP8266 ESP-NOW + WiFi coexistence on D1 Mini under load
