# Software roadmap

Incremental bring-up before the production app. **Do not skip phases** — each test de-risks the next.

Status legend: `⬜ planned` · `🔄 in progress` · `✅ done`

---

## TEST 1 — Sensor bring-up `✅`

**Status:** Complete — **human verified** on bench 2026-07-05  
**Verified by:** Operator (Dorinda hardware)  
**Evidence:** [test-log.md](test-log.md#test-1--sensor-bring-up)

**Path:** `firmware/tests/test1_sensors/`

**Goal:** Verify analog and digital sensors on the bench with **no WiFi**, **no PWM to fan** (or fan disconnected).

### Behaviour

- Read potentiometer on A0 continuously
- Read DS18B20 on D2
- Print once per second to Serial (115200 baud):

```
Pot=512 ADC=512 Temp=32.4C
```

### Pass criteria

- [x] Pot reading moves smoothly from min→max when knob turned
- [x] Document observed ADC range (`min=5 max=1024`) in [test-log.md](test-log.md)
- [x] Temperature plausible vs room / finger test
- [x] Serial output stable at 1 Hz, no crash after flash *(human verified)*
- [ ] `Temp=---C` if sensor unplugged — *deferred; not required for TEST 1 sign-off*

**Human verification (2026-07-05):** Pot sweeps 5→1024 on physical knob; DS18B20 reads stable plausible temps over serial @ 115200. Bench sign-off recorded in [test-log.md](test-log.md).

### Libraries

- OneWire
- DallasTemperature

### Notes for later phases

Record pot noise (jitter at fixed position). Main app override-drop logic needs a **deadband** — TEST 1 measured ±1 ADC count; start deadband at **~30–50 counts** (3–5 % of span).

---

## TEST 2 — WiFi + live pot in browser `✅`

**Status:** Complete — **human verified** 2026-07-05 (AP mode, pot + temp web UI stable)

**Path:** `firmware/tests/test2_wifi_pot/`

**Goal:** Prove WiFi + async web server can serve live pot value on the boat (no shore router).

### Behaviour

- **Default `WIFI_MODE=AP`** — ESP8266 creates its own network (`secrets.env`: SSID + password); phone joins and opens `http://192.168.4.1/`
- **`WIFI_MODE=STA`** (optional) — join existing network via BSP `bsp_wifi_connect()` for home bench
- Embedded web server (ESPAsyncWebServer)
- Page shows pot ADC + percent, auto-refresh 1 s
- Serial mirrors pot value at 1 Hz:

```
AP OK ssid=Dorinda-Fan ip=192.168.4.1
AP ip=192.168.4.1 clients=1 Pot=512 (50%)
```

### Pass criteria

- [x] AP starts reliably after power cycle (boat default)
- [x] Phone joins AP and browser stable with pot + temp (human verified)
- [x] Serial and web values match within one refresh period
- [x] No PWM output (D5 held at 0 in TEST 2)

### WiFi / BSP usage

- **AP mode** — `WiFi.softAP()` (no router on Dorinda)
- **STA mode** — `bsp/wifi.h` for home testing
- Credentials in repo-root `secrets.env` (`WIFI_MODE`, `WIFI_SSID`, `WIFI_PASSWORD`)

---

## TEST 3 — PWM → current calibration matrix `✅`

**Path:** `firmware/tests/test3_pwm_matrix/`

**Goal:** Build and maintain the lookup table used by the main app to **estimate fan current** from PWM duty.

**Production path:** Matrix measured on **lab bench** 2026-07-05 — see [test-log.md](test-log.md#test-3--pwm--cal-matrix-). Main app ships embedded defaults; TEST 3 remains for regression and re-cal on hardware.

### Workflow (operator — optional on-device)

1. Flash TEST 3 firmware
2. Open calibration web page (`/cal`)
3. For each PWM step, measure mA and save row
4. Copy updated knots to `firmware/data/cal_matrix.json` and embedded defaults in `cal_matrix` lib

### Pass criteria

- [x] Matrix measured and in repo (`bench_lab`)
- [x] Linear interpolation in `cal_matrix` lib
- [x] Fan responds at PWM steps (bench)
- [x] PWM ramps (TEST 3: 200 ms; main v1.0: 1 s steps)
- [ ] On-boat re-verify after install — pending

### Deliverable for main app

`firmware/data/cal_matrix.json` + `cal::load_embedded_defaults()` ([ADR-0016](decisions/0016-embedded-cal-matrix-main.md)).

---

## Main application `✅` (v1.0)

**Path:** `firmware/apps/main/` — `make main-dev` (from repo root)  
**Tag:** `v1.0` (2026-07-06)

### Functional requirements (v1.0)

| # | Requirement | Status |
|---|-------------|--------|
| M1 | WiFi AP/STA on boot (`dfc_wifi`) | ✅ |
| M2 | Web server + mobile dashboard | ✅ |
| M3 | Live pot (raw + %) | ✅ |
| M4 | DS18B20 temp (5 s tick) | ✅ |
| M5 | Slider remote setpoint (0–100 %) | ✅ |
| M6 | Remote clears when pot moves >4 % | ✅ |
| M7 | Ramped PWM @ 18 kHz (1 s / 6 % steps) | ✅ |
| M8 | Estimated current from bench matrix | ✅ |
| M9 | `/api/status` JSON | ✅ |
| M10 | Boot-safe: fan off until pot **or** remote arms | ✅ |
| M11 | `POST /api/climate` for forepeak | ✅ |
| M12 | Presets OFF / Silent / FULL | ✅ |

### Control logic (implemented)

```
control::output_percent(pot_pct)  // 0 until boot armed (pot or prior remote)
control::set_remote(pct, pot)     // arm_boot() + remote mode
fan_control_set_new_setpoint(output)
fan_control_loop()                // once per CONTROL_INTERVAL_MS (1 s)
est_ma = cal::interpolate_ma(pwm)
```

`DEADBAND` = **4 %** (`POT_DEADBAND_PCT`). See [ADR-0007](decisions/0007-override-drop-on-pot-move.md).

### Loop timing

1 s control · 5 s temperature · 1 s serial — [ADR-0014](decisions/0014-cooperative-loop-timing.md).

### Safety requirements (main app V2) `⬜`

| Condition | Action |
|-----------|--------|
| Temp > 70 °C | Force PWM = 100 %, warning on UI |
| Temp > 85 °C | **FAULT**: PWM = 0, latch until reset |
| DS18B20 fault | Show `---` |

See [ADR-0008](decisions/0008-safety-thresholds-v2.md).

### Web UI (v1.0)

- Pot, temp, fan %, est. mA
- Remote slider + back to manual
- **OFF · Silent · FULL** presets
- Mobile-friendly layout

---

## Future — ESP-NOW auto mode `⬜`

**Separate repo planned:** remote climate node (temp + humidity).

| Feature | Description |
|---------|-------------|
| Transport | ESP-NOW peer link, not WiFi TCP |
| Control | `AUTO` mode: PID or rule-based fan speed from remote readings |
| Fallback | Loss of ESP-NOW link → revert to pot control |

Architecture stub in main app from day one — see [architecture.md](architecture.md).

---

## Suggested implementation order

```
TEST 1 ──► TEST 2 ──► TEST 3 (bench cal) ──► Main v1.0 ──► Safety V2 ──► ESP-NOW
   │           │              │
   └─ no BSP    └─ dfc_wifi    └─ TEST3 on-device cal optional
```
