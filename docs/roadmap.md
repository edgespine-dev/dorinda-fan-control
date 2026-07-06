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

## TEST 3 — PWM → current calibration matrix `🔄`

**Path:** `firmware/tests/test3_pwm_matrix/`

**Goal:** Build the lookup table used by the main app to **estimate fan current** from PWM duty.

### Workflow (operator)

1. Flash TEST 3 firmware
2. Open calibration web page
3. For each PWM step (suggested: 0, 64, 128, 192, 256, 384, 512, 768, 1023):
   - Set PWM via web UI
   - Measure actual current with multimeter / clamp meter
   - Enter measured mA in the web form
4. Submit → firmware stores row and **prints full matrix to Serial**

### Serial output example

```
Matrix updated (9 rows):
  PWM=    0  ->  12 mA
  PWM=  128  -> 145 mA
  ...
```

### Pass criteria

- [ ] Matrix persists across reboot (SPIFFS / LittleFS / NVS)
- [ ] Serial dump on every matrix edit
- [ ] Interpolation between points documented (linear between knots)
- [ ] Fan audibly/spin-wise responds at each PWM step
- [ ] PWM output **ramps** in **200 ms** steps (≤64 counts/step) — limits inrush current

### Deliverable for main app

JSON or struct array consumed by main app:

```json
[
  {"pwm": 0, "current_ma": 12},
  {"pwm": 128, "current_ma": 145}
]
```

File location (planned): `firmware/data/current_matrix.json` or NVS namespace `cal`.

---

## Main application `🔄`

**Path:** `firmware/apps/main/` — `make main-dev`

### Functional requirements

| # | Requirement |
|---|-------------|
| M1 | WiFi connect on boot (BSP) |
| M2 | Web server with single control page |
| M3 | Display live pot position (raw + %) |
| M4 | Display DS18B20 enclosure temperature |
| M5 | Slider sets PWM override (0–100 %) |
| M6 | **Override drop:** physical pot movement beyond deadband cancels override |
| M7 | PWM on D5 follows override if active, else pot — **ramped** in 200 ms steps (≤64 PWM counts/step), never instant jump |
| M8 | Show **estimated current** from TEST 3 matrix + current PWM |
| M9 | API endpoint `/api/status` JSON for debugging |

### Override logic (draft)

```
if override_active:
    if abs(pot - pot_at_override_lock) > DEADBAND:
        override_active = false

output_pct = override_active ? slider_pct : drivers::pot_read_value_percentage()
drivers::fan_control_set_new_setpoint(output_pct)
drivers::fan_control_loop()   // ramp 200 ms / 6 % steps
estimated_ma = interpolate(matrix, drivers::fan_control_get_pwm_raw())
```

`DEADBAND` tuned from TEST 1 logs (start with 3–5 % of ADC range).

### Safety requirements (main app V2)

| Condition | Action |
|-----------|--------|
| Temp > 70 °C | Force PWM = 100 %, show warning on web UI |
| Temp > 85 °C | Enter **FAULT**: PWM = 0, latch until reset, Serial alarm |
| DS18B20 fault | Show `---`, optional conservative fan speed |

### Web UI (minimum)

- Pot gauge / numeric
- Temperature
- Current estimate (mA)
- PWM slider + “Override active” indicator
- Mobile-friendly layout (boat use case)

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
TEST 1 ──► TEST 2 ──► TEST 3 ──► Main app (M1–M9) ──► Safety V2 ──► ESP-NOW
   │           │           │
   └─ no BSP    └─ BSP     └─ BSP + web + NVS
      WiFi         WiFi
```
