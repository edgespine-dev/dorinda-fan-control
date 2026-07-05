# Architecture

How **dorinda-fan-control** is structured, how it uses **bakerbug-firmware BSP**, and what to preserve for future ESP-NOW auto mode.

**Rationale for major choices:** [docs/decisions/](decisions/)

---

## Design principles

1. **Test layers in order** — sensors, then network, then calibration, then product logic
2. **BSP for platform, local code for product** — do not fork WiFi/TLS into this repo
3. **Thin main loop** — hardware polling, control policy, and HTTP handled in separate modules
4. **Calibration is data** — PWM→current matrix is not hard-coded; it comes from TEST 3
5. **Future ESP-NOW** — control policy accepts multiple inputs (local pot, web override, remote climate)

---

## Dependency on bakerbug-firmware

This repo **does not duplicate** platform code. It vendors [bakerbug-firmware](https://github.com/BakerStack/bakerbug-firmware) as a git submodule:

```
vendor/bakerbug-firmware/
└── firmware/
    ├── bsp/
    │   ├── include/bsp/     ← headers (wifi, gpio, log, …)
    │   └── esp8266/         ← ESP8266 implementations
    └── lib/secrets/         ← optional: WiFi credential injection pattern
```

### What we use from BSP

| Module | Used in | Skip |
|--------|---------|------|
| `bsp/wifi.h` | TEST 2, TEST 3, main | — |
| `bsp/log.h` | All | — |
| `bsp/gpio.h` | If we abstract PWM later | Optional in TEST 1 |
| `mqtt/*` | — | **Not used** |
| SPIRE / provisioning | — | **Not used** |

### PlatformIO linkage (planned)

```ini
[env:esp8266]
platform = espressif8266
board = d1_mini
framework = arduino

lib_extra_dirs =
    vendor/bakerbug-firmware/firmware/lib

lib_deps =
    bsp/esp8266
    ; secrets — only if using compile-time WiFi injection
```

Pin a submodule commit when a release is tagged so builds are reproducible.

---

## Firmware module map (main app target)

```
┌─────────────────────────────────────────────────────────┐
│  main.cpp          setup(), loop orchestration          │
├─────────────────────────────────────────────────────────┤
│  SensorService     pot ADC filter, DS18B20 read         │
│  FanOutput         PWM write, safety clamp              │
│  ControlPolicy     pot vs override vs auto (future)     │
│  CurrentEstimate   matrix lookup + interpolation       │
│  WebServer         HTTP page + /api/status              │
│  CalibrationStore  load/save PWM→mA matrix (NVS/FS)    │
├─────────────────────────────────────────────────────────┤
│  bakerbug BSP      WiFi, logging                        │
└─────────────────────────────────────────────────────────┘
```

Each TEST program implements a **subset** of these modules — TEST 1 only `SensorService`, etc.

---

## Control modes (enum — stub early)

```cpp
enum class ControlMode {
    Manual,     // pot drives PWM (default)
    Override,   // web slider drives PWM until pot moves
    Auto,       // future: ESP-NOW climate node
    Fault       // over-temp latched shutdown
};
```

Main app V1 implements `Manual`, `Override`, `Fault`. `Auto` returns `Manual` behaviour until climate node exists.

---

## Data: PWM → current matrix

Produced by **TEST 3**, consumed by **main app**.

```cpp
struct CalPoint { uint16_t pwm; uint16_t current_ma; };
```

- Storage: SPIFFS/LittleFS JSON or NVS blob
- Lookup: linear interpolation between sorted PWM knots
- Edge cases: below min knot → min mA; above max → max mA

---

## Web API (main app, draft)

| Method | Path | Purpose |
|--------|------|---------|
| GET | `/` | Control page |
| GET | `/api/status` | JSON: pot, temp, pwm, override, current_ma, mode |
| POST | `/api/override` | `{ "pwm": 0–1023 }` enable override |
| DELETE | `/api/override` | Clear override |

TEST 3 may reuse the same web stack with different routes for matrix editing.

---

## Future: ESP-NOW climate node

```
┌──────────────────┐         ESP-NOW          ┌──────────────────┐
│  Climate node    │  temp, humidity, seq     │  Fan controller  │
│  (future repo)   │ ───────────────────────► │  (this repo)     │
│  ESP8266/32      │                          │  Dorinda bay     │
└──────────────────┘                          └──────────────────┘
```

**Why ESP-NOW:** sensor can live in the **cabin** or another compartment without running cables to the fan controller. WiFi remains for phone browser access to the fan unit.

**Integration plan:**

1. Add `EspNowReceiver` module with callback → updates `ClimateReading`
2. Extend `ControlPolicy` with humidity/temp setpoints
3. Keep manual override priority: pot movement always wins over auto

Peer MAC pairing stored in NVS. Out of scope until main app V1 ships.

---

## Repository boundaries

| In this repo | In bakerbug-firmware | Future repo |
|--------------|----------------------|-------------|
| Fan control logic | BSP, shared drivers | Climate ESP-NOW node |
| Web UI assets | MQTT stack | — |
| Calibration matrix | — | — |
| Dorinda-specific HW docs | Generic ESP8266 BSP | Remote sensor HW |
