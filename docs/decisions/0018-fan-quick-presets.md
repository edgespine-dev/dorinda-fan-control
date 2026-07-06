# ADR-0018: Quick presets — OFF, Silent, FULL

**Status:** Accepted  
**Date:** 2026-07-06

## Context

Helm and phone UI need **one-tap** fan levels without dragging a slider. “Silent” should mean the minimum useful crawl on the 12 V bus — historically ~**1 V** (~327 mA per bench matrix), not fully off.

## Decision

Three preset buttons on the main web UI (below slider / manual controls):

| Preset | Percent | Rationale |
|--------|---------|-----------|
| **OFF** | 0 | Fan stopped |
| **Silent** | `SILENT_PERCENTAGE` | `(1 V / 12 V) × 100` → **8 %** (rounded) |
| **FULL** | 100 | Maximum airflow |

Constants in `firmware/apps/main/src/main.cpp`:

```cpp
constexpr int FAN_BUS_NOMINAL_V = 12;
constexpr int SILENT_PERCENTAGE = (1 * 100 + FAN_BUS_NOMINAL_V / 2) / FAN_BUS_NOMINAL_V;
```

**API:** `GET /api/presets` → `{"off":0,"silent":8,"full":100}`  
**Action:** each button calls existing `PUT /api/fan/setpoint` (remote mode, ADR-0007 pot override still applies).

## Consequences

**Positive:**

- Single source of truth for Silent voltage mapping
- Forepeak node can read `/api/presets` instead of hard-coding 8 %

**Negative:**

- Silent % is nominal; actual bus voltage varies with battery state — est_mA from matrix still approximate

**Follow-ups:**

- Rename “Silent” in UI if boat installs a different nominal bus (e.g. 24 V — would need config)
