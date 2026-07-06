# Calibration data

## `cal_matrix.json`

PWM → fan current lookup table for TEST 3 and main app.

| Field | Meaning |
|-------|---------|
| `meta.source` | `bench_lab` — measured on lab bench, not aboard Dorinda under way |
| `meta.location` | Dorinda electronics cube wiring on bench |
| `meta.date` | Measurement session |
| `meta.supply_v` | Nominal 12 V supply, not charging |
| `rows` | PWM duty (0–1023) → current (mA) |

Full traceability log: [docs/test-log.md](../../docs/test-log.md#test-3--pwm--cal-matrix-).

Upload to device (optional — firmware seeds same defaults on first boot):

```bash
cd firmware && pio run -e test3 -t uploadfs
```
