# Architecture

How **dorinda-fan-control** is structured, how it uses **bakerbug-firmware**, and what to preserve for future ESP-NOW auto mode.

**Rationale for major choices:** [docs/decisions/](decisions/) — especially [ADR-0014](decisions/0014-cooperative-loop-timing.md) (loop timing), [ADR-0015](decisions/0015-boot-safe-fan-off.md) (boot safety), [ADR-0017](decisions/0017-dfc-wifi-shared-boot.md) (WiFi).

**Release:** `v1.0` — main app on ESP8266 with REST + pot + bench cal matrix.

---

## Design principles

1. **Test layers in order** — sensors, then network, then calibration, then product logic ([ADR-0004](decisions/0004-incremental-test-phases.md))
2. **BSP for platform, drivers for chips, Dorinda libs for product** — do not fork WiFi into this repo
3. **Cooperative scheduling** — not real-time; 1 s control, 5 s temp; `yield()` for radio ([ADR-0014](decisions/0014-cooperative-loop-timing.md))
4. **Calibration is data** — bench-measured PWM→mA matrix; embedded on main ([ADR-0016](decisions/0016-embedded-cal-matrix-main.md))
5. **Boot-safe fan** — D5 held off until pot **or** remote arms ([ADR-0015](decisions/0015-boot-safe-fan-off.md))
6. **Future ESP-NOW** — control policy accepts pot, remote HTTP, and (later) climate node ([ADR-0009](decisions/0009-esp-now-future-climate-node.md))

---

## Layer map (v1.0)

```
dorinda-fan-control                         bakerbug-firmware (submodule)
─────────────────────                       ─────────────────────────────
firmware/apps/main/src/main.cpp    ≈        Application
  HTTP routes, loop ticks, presets
       │
firmware/lib/control/              ≈        Product policy (manual / remote)
firmware/lib/cal_matrix/           ≈        PWM → mA lookup
firmware/lib/dfc_wifi/             ≈        Dorinda WiFi boot (secrets → AP/STA)
firmware/lib/hw/pins.h             ≈        Board pin map only
       │
vendor/…/lib/drivers/              =        ds18b20, potentiometer, fan_control
vendor/…/bsp/                      =        wifi, log
       │
Arduino / ESP8266                  =        Arduino / ESP8266
```

### What goes where

| Code | Location | Example |
|------|----------|---------|
| WiFi connect, softAP, logging | **bakerbug BSP** | `bsp_wifi_softap_begin()`, `log_printf` |
| Chip / actuator drivers | **bakerbug `drivers/`** | `drivers::fan_control_*`, `drivers::pot_*` |
| Pin map | **dorinda `hw/pins.h`** | `FAN_PWM` → D5 |
| Manual vs remote, boot arm | **`control/`** | `set_remote()` → `arm_boot()`, `boot_armed()` |
| Cal matrix | **`cal_matrix/`** | `cal::interpolate_ma()` |
| WiFi boot glue | **`dfc_wifi/`** | `dfc_wifi_boot()` |
| HTTP, UI, tick orchestration | **`apps/main/`** | `setup_web()`, `loop()` |

---

## Main loop (v1.0)

Not a real-time system. The loop body is intentionally idle between ticks.

| Tick | Period | Work |
|------|--------|------|
| Control | 1 s | Pot read, `control::output_percent`, fan setpoint + one ramp step |
| Temperature | 5 s | DS18B20 (blocking OneWire — keep away from AP bring-up) |
| Report | 1 s | Serial status line |
| Always | every `loop()` | `yield()` for WiFi / AsyncTCP |

**Boot sequence:** `fan_control_force_off` → init drivers → `dfc_wifi_boot()` → `setup_web()` → ready.

Fan PWM driver init (`fan_control_init`) runs only after **boot arm**:

| Arm path | Trigger |
|----------|---------|
| **Pot** | Pot moves >4 % from first control sample |
| **Remote** | `PUT /api/fan/setpoint`, presets, or `POST /api/climate` with `fan_percent` → `control::arm_boot()` |

Until armed, manual pot path returns 0 %; remote commands arm and apply in one step.

---

## Control modes

Implemented in `firmware/lib/control/`:

```cpp
enum class Mode { Manual, Remote };
```

| Mode | Source | Clears when |
|------|--------|-------------|
| **Manual** | Pot → `output_percent` | — |
| **Remote** | `PUT /api/fan/setpoint`, `POST /api/climate` | Pot moves > deadband ([ADR-0007](decisions/0007-override-drop-on-pot-move.md)); also **arms** fan if still boot-latched ([ADR-0015](decisions/0015-boot-safe-fan-off.md)) |

`Fault` / thermal safety ([ADR-0008](decisions/0008-safety-thresholds-v2.md)) — **planned V2**, not in v1.0.

---

## Data: PWM → current matrix

Bench lab measurement 2026-07-05 — see [test-log.md](test-log.md#test-3--pwm--cal-matrix-).

- **Canonical file:** `firmware/data/cal_matrix.json` (`meta.source=bench_lab`)
- **Main app:** `cal::load_embedded_defaults()` — no LittleFS ([ADR-0016](decisions/0016-embedded-cal-matrix-main.md))
- **TEST 3:** LittleFS + `/cal` editor for regression

```cpp
struct Row { uint16_t pwm; uint16_t current_ma; };
```

Lookup: linear interpolation between sorted PWM knots.

---

## REST API (main app)

Phone, browser, and forepeak ESP32 use HTTP JSON on the fan controller (AP default `dfan` @ `192.168.4.1`).

| Method | Path | Body | Purpose |
|--------|------|------|---------|
| GET | `/` | — | Mobile dashboard, slider, presets |
| GET | `/api/status` | — | mode, pot, fan, pwm, est_mA, temp, climate |
| GET | `/api/presets` | — | `{"off":0,"silent":8,"full":100}` ([ADR-0018](decisions/0018-fan-quick-presets.md)) |
| GET | `/api/fan` | — | Fan subset: mode, setpoint, current |
| PUT | `/api/fan/setpoint` | `{"percent":0-100}` | Remote control |
| DELETE | `/api/fan/remote` | — | Back to manual pot |
| POST | `/api/climate` | see below | Forepeak: readings + optional `fan_percent` |

**POST /api/climate** (forepeak ESP32):

```json
{
  "temp_c": 22.5,
  "humidity_pct": 55,
  "co2_ppm": 800,
  "fan_percent": 42
}
```

Climate block on `/api/status` expires after **120 s** without refresh. Auto CO₂/temp logic is **future** — node may send `fan_percent` directly.

CORS: `Access-Control-Allow-Origin: *` on JSON routes.

---

## Web UI (v1.0)

- Live pot %, fan out %, bay temp, estimated mA
- Slider + **Apply remote** / **Back to pot**
- **Quick presets:** OFF · Silent (8 %) · FULL ([ADR-0018](decisions/0018-fan-quick-presets.md))
- `fetch('/api/status')` every 2 s

---

## Future: ESP-NOW climate node

```
┌──────────────────┐         ESP-NOW          ┌──────────────────┐
│  Climate node    │  temp, humidity, seq     │  Fan controller  │
│  (future repo)   │ ───────────────────────► │  (this repo)     │
│  ESP8266/32      │                          │  Dorinda bay     │
└──────────────────┘                          └──────────────────┘
```

**Near-term alternative (v1.0):** forepeak ESP32 posts to **`POST /api/climate`** over WiFi while joined to `dfan` AP — no ESP-NOW yet.

**Integration plan:**

1. Add `EspNowReceiver` or extend HTTP climate with auto policy
2. Extend `ControlPolicy` with humidity/temp setpoints
3. Pot movement always wins over auto ([ADR-0007](decisions/0007-override-drop-on-pot-move.md))

Details: [ADR-0009](decisions/0009-esp-now-future-climate-node.md)

---

## Repository boundaries

| In this repo | In bakerbug-firmware | Future repo |
|--------------|----------------------|-------------|
| `control`, `cal_matrix`, `dfc_wifi`, main app | BSP, shared `drivers/` | Climate ESP-NOW node |
| Web UI (embedded HTML) | MQTT stack (unused) | — |
| Bench cal JSON + embedded copy | — | — |
| Dorinda HW docs | Generic ESP8266 BSP | Remote sensor HW |
