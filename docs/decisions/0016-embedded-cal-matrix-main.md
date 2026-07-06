# ADR-0016: Main app uses embedded bench matrix (no LittleFS)

**Status:** Accepted  
**Date:** 2026-07-06

## Context

TEST 3 stores the PWM→mA matrix in **LittleFS** and exposes a `/cal` web editor. On ESP8266, mounting a filesystem during WiFi bring-up has been observed to correlate with **unreliable softAP** behaviour.

Production calibration was completed on the **lab bench** (2026-07-05) and checked into `firmware/data/cal_matrix.json` with `meta.source=bench_lab`.

## Decision

- **Main app** loads the matrix via `cal::load_embedded_defaults()` — compile-time table in `cal_matrix`, no filesystem mount.
- **TEST 3** retains LittleFS + `/cal` for regression and future re-measurement.
- Canonical data: `firmware/data/cal_matrix.json` (traceability); embedded copy must stay in sync when matrix changes.

## Consequences

**Positive:**

- Simpler flash layout for main (`[env:main]` has no `board_build.filesystem`)
- Fewer moving parts at boot; matrix always present
- Bench traceability via JSON in repo

**Negative:**

- Changing cal on boat requires reflash or returning to TEST 3 workflow
- Two copies of default rows (JSON + C++ `kDefaultRows`) — update both when re-benchmarking

**Follow-ups:**

- Script or CI check that JSON and embedded defaults match (optional)
