# Calibration data

## `cal_matrix.json`

**Canonical** PWM → fan current lookup for Dorinda v1.0.

| Field | Meaning |
|-------|---------|
| `meta.source` | `bench_lab` — lab bench, not aboard Dorinda under way |
| `meta.location` | Dorinda electronics cube on bench |
| `meta.date` | Measurement session (2026-07-05) |
| `meta.supply_v` | ~12 V, not charging |
| `rows` | PWM (0–1023) → current (mA) |

**Main app:** embeds the same knots via `cal::load_embedded_defaults()` — no filesystem ([ADR-0016](../../docs/decisions/0016-embedded-cal-matrix-main.md)).  
**TEST 3:** can load/save via LittleFS + `/cal` UI.

When re-measuring:

1. Update this JSON
2. Sync `kDefaultRows` in `firmware/lib/cal_matrix/src/matrix.cpp`
3. Log in [docs/test-log.md](../../docs/test-log.md)

Upload to TEST 3 device (optional):

```bash
cd firmware && pio run -e test3 -t uploadfs
```

Traceability: [docs/test-log.md](../../docs/test-log.md#test-3--pwm--cal-matrix-).
