# Bench test log

Hardware-specific observations from bring-up on the Dorinda D1 Mini board.

Status legend: `⬜ pending` · `✅ human verified` · `🔄 in progress`

---

## TEST 1 — Sensor bring-up `✅`

**Date:** 2026-07-05  
**Firmware:** `firmware/tests/test1_sensors/`  
**Board:** Wemos D1 Mini (ESP8266EX), MAC `ec:64:c9:f0:f2:a3`, 4 MB flash  
**Port:** `/dev/cu.usbserial-14420` (CH340)

### Human verification sign-off

| Item | Result |
|------|--------|
| **Verifier** | Operator — live bench on Dorinda D1 Mini |
| **Method** | `make test1-upload` + `make test1-monitor` @ 115200 |
| **Pot** | Physical sweep CCW→CW; smooth response, no jumps |
| **ADC range** | min=**5**, max=**1024** |
| **Temperature** | DS18B20 stable, plausible room readings |
| **Stability** | 1 Hz serial, no crash during session |
| **Sign-off** | **PASS — TEST 1 complete** |

> Unplugged-sensor path (`Temp=---C`) not exercised at sign-off — deferred.

### Flash & serial

| Check | Result |
|-------|--------|
| `make test1-upload` | OK (~29 s) |
| Boot banner | `TEST 1 — pot + DS18B20 (no WiFi, no fan PWM)` |
| Report rate | 1 Hz @ 115200 baud |
| Format | `Pot=<adc> Temp=<°C>` |

### Potentiometer (A0, B50K)

Full sweep — human verified 2026-07-05:

| Metric | Value |
|--------|-------|
| ADC min (CCW) | **5** |
| ADC max (CW) | **1024** |
| Jitter at fixed position | ±1 count |
| Deadband hint (3–5 % of ~1019 span) | **~30–50 ADC counts** |

### DS18B20 (D2)

| Check | Result |
|-------|--------|
| Room temp | ✅ Human verified — stable, plausible |
| Unplugged sensor (`Temp=---C`) | ⬜ Not tested (deferred) |

---

## TEST 2 — WiFi + live pot `✅`

**Date:** 2026-07-05  
**Firmware:** `firmware/tests/test2_wifi_pot/`

| Item | Result |
|------|--------|
| WiFi | AP mode (`dfan`), phone join OK |
| Web UI | Pot %, ADC, temp — stable with 2 clients after async fix |
| Serial | Matches cached sensors @ 1 Hz |
| Sign-off | **PASS — TEST 2 complete** |

---

## TEST 3 — PWM + cal matrix `✅`

**Date:** 2026-07-05  
**Sign-off:** Matrix measured **bench lab** (below). TEST 3 kept for regression; use **main app** on boat.

### Calibration matrix — bench lab (traceability)

| Traceability | Value |
|--------------|-------|
| **Where** | Lab bench — Dorinda electronics cube (not installed on boat) |
| **When** | 2026-07-05 |
| **Source** | `bench_lab` (see `firmware/data/cal_matrix.json` → `meta`) |
| **Supply** | ~12 V, house bank at rest — **not charging** |
| **Instrument** | Clamp meter |
| **PWM mapping** | V_fan / 12 × 1023 |

| ~V (fan) | Measured A | mA | PWM |
|----------|------------|-----|-----|
| 1.01 | 0.327 | 327 | 86 |
| 2.4 | 0.632 | 632 | 205 |
| 4.8 | 1.507 | 1507 | 410 |
| **5.0** | **1.7** | **1700** | **426** |
| 6.0 | 2.08 | 2080 | 512 |
| 7.2 | 2.6 | 2600 | 614 |
| 9.6 | 4.0 | 4000 | 819 |
| 12.0 | 5.56 | 5560 | 1023 |

**Notes:** 5 V / ~1.7 A = anchor sweet spot (~42 % PWM). Est. mA most accurate when not charging.

| Item | Result |
|------|--------|
| Matrix in repo | ✅ |
| Bench measurement | ✅ Lab 2026-07-05 |
| On-boat install | ⬜ Pending |

---

## Main app `🔄`

**Path:** `firmware/apps/main/` — `make main-dev`  
REST API: [architecture.md](architecture.md#rest-api-main-app)
