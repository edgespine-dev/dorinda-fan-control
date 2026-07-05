# ADR-0005: ESP8266 Wemos D1 Mini as sole MCU (V1)

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Hardware is already wired around a **Wemos D1 Mini** (ESP8266). Pin choices (D2 OneWire, D5 PWM, A0 ADC) are fixed on the PCB/enclosure design. ESP32 BSP exists in bakerbug-firmware but is not required for V1 workloads (single fan, one temp sensor, lightweight web UI).

## Decision

V1 targets **ESP8266 / D1 Mini only**. Framework: **Arduino** via PlatformIO. ESP32 port deferred unless RAM/CPU limits appear.

## Consequences

**Positive:**

- Matches built hardware
- Mature Arduino-ESP8266 PWM/ADC support
- Reuses `bsp/esp8266` directly

**Negative:**

- ESP8266 has limited ADC (single pin A0) — already sufficient
- No BLE; ESP-NOW still supported on ESP8266 for future climate node

**Follow-ups:**

- Revisit ESP32 only if concurrent ESP-NOW + heavy web UI exceeds headroom
