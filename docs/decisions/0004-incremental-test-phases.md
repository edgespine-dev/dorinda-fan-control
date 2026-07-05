# ADR-0004: Incremental test phases (TEST 1 → 2 → 3) before main app

**Status:** Accepted  
**Date:** 2026-07-05

## Context

The hardware combines **analog pot**, **OneWire temperature**, **12 V PWM fan switching**, and **WiFi web UI**. Debugging everything at once on a boat bench is painful. We need confidence in each layer before stacking the next.

## Decision

Ship firmware in **locked phases**:

1. **TEST 1** — Serial only: pot + DS18B20 (no WiFi, no fan PWM or fan disconnected)
2. **TEST 2** — WiFi + web page showing pot (no PWM)
3. **TEST 3** — Web UI to build PWM→current calibration matrix
4. **Main app** — Full controller using matrix from TEST 3

Each phase has explicit pass criteria in [roadmap.md](../roadmap.md).

## Consequences

**Positive:**

- Failures isolate to one subsystem
- TEST 1 documents pot ADC range/noise for override deadband
- TEST 3 produces real data for current estimation in main app

**Negative:**

- More folders and flash cycles upfront
- Temptation to skip ahead must be resisted

**Follow-ups:**

- Do not merge main app until TEST 1–3 pass on **this** hardware
