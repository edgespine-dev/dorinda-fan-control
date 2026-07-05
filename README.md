# Dorinda Fan Control

Smart 12 V enclosure fan controller for **Dorinda** (sailboat electronics bay). Built on **ESP8266**, with incremental bring-up tests, a calibration workflow for PWM→current mapping, and a production web UI for manual control with safety overrides.

> **If you are reading this after a long break:** start with [Quick orientation](#quick-orientation). **TEST 1–2 done** (human verified 2026-07-05) — next up is **TEST 3** (PWM + cal matrix on bench with 12 V).

[![Platform](https://img.shields.io/badge/platform-ESP8266-blue)]()
[![Framework](https://img.shields.io/badge/framework-Arduino-orange)]()
[![BSP](https://img.shields.io/badge/BSP-bakerbug--firmware-green)]()

---

## Quick orientation

| What | Where |
|------|-------|
| Hardware wiring & pin map | [docs/hardware.md](docs/hardware.md) |
| CAD / mechanical | [cad/](cad/) *(Autodesk exports — add when ready)* |
| Software architecture & BSP dependency | [docs/architecture.md](docs/architecture.md) |
| Test phases & main app spec | [docs/roadmap.md](docs/roadmap.md) |
| **Design decision log (ADR)** | [docs/decisions/](docs/decisions/) |
| Firmware source (when implemented) | [firmware/](firmware/) |

**Build philosophy:** small, verifiable steps. Each TEST program proves one layer before the next. The main app only ships after TEST 1–3 pass on real hardware.

**External dependency:** this repo consumes the **Board Support Package (BSP)** from [bakerbug-firmware](https://github.com/BakerStack/bakerbug-firmware) for WiFi and other platform abstractions — not MQTT or cloud stack.

---

## What this project does

```
┌─────────────────────────────────────────────────────────────┐
│  Operator (browser on phone/laptop)                         │
│  • See pot position, temperature, estimated current         │
│  • Override fan speed with slider                           │
│  • Override clears when physical pot is moved                │
└──────────────────────────┬──────────────────────────────────┘
                           │ HTTP (WiFi)
┌──────────────────────────▼──────────────────────────────────┐
│  Wemos D1 Mini (ESP8266)                                    │
│  • Read B50K pot (A0)                                       │
│  • Read DS18B20 enclosure temp (D2)                         │
│  • PWM fan via MOSFET module (D5)                           │
│  • Lookup PWM → current from calibration matrix (TEST 3)    │
└──────────────────────────┬──────────────────────────────────┘
                           │ 12 V low-side switch
┌──────────────────────────▼──────────────────────────────────┐
│  12 V fan + XY-MOS MOSFET module                            │
└─────────────────────────────────────────────────────────────┘

Future: remote ESP8266/ESP32 climate node ──ESP-NOW──► auto mode
```

---

## Hardware summary

**Controller:** Wemos D1 Mini (ESP8266 / ESP-12F)

**Power**

- 12 V supply → buck converter → **5 V** → D1 Mini `5V` pin
- **Common GND** shared between: 12 V−, buck GND, D1 Mini GND, MOSFET module GND

**Fan driver:** XY-MOS V921-style module, low-side switching

| MOSFET module | Connection |
|---------------|------------|
| GND | D1 Mini GND |
| TRIG/PWM | D1 Mini **D5** |

**Temperature:** DS18B20 on **D2**, 4.7 kΩ pull-up DATA→3V3

| DS18B20 | Connection |
|---------|------------|
| VDD | 3V3 |
| GND | GND |
| DATA | D2 |

**Manual control:** B50K potentiometer on **A0**

| Pot pin | Connection |
|---------|------------|
| Outer 1 | 3V3 |
| Outer 2 | GND |
| Wiper | A0 |

Full pin table and wiring notes: [docs/hardware.md](docs/hardware.md)

---

## Software roadmap

Development proceeds in locked order. Do not skip ahead.

| Phase | Folder | Purpose | Status |
|-------|--------|---------|--------|
| **TEST 1** | `firmware/tests/test1_sensors/` | Pot + DS18B20 → Serial | ✅ **Done** — human verified 2026-07-05 ([log](docs/test-log.md)) |
| **TEST 2** | `firmware/tests/test2_wifi_pot/` | WiFi AP + pot/temp web | ✅ Done ([log](docs/test-log.md)) |
| **TEST 3** | `firmware/tests/test3_pwm_matrix/` | Pot→PWM + cal matrix | 🔄 Ready to flash |
| **Main app** | `firmware/apps/main/` | Production controller | ⬜ Planned |

Detailed requirements for each phase: [docs/roadmap.md](docs/roadmap.md)

---

## Main application (target behaviour)

When TEST 1–3 are complete, the main app provides:

- **WiFi** — connect using credentials from build-time secrets (same pattern as bakerbug-firmware)
- **Web server** — single responsive page:
  - Live **pot position** (ADC + normalized %)
  - **Slider override** for fan PWM (0–100 %)
  - **Override drop:** any physical pot movement cancels override; pot regains authority
  - **PWM output** to MOSFET (D5)
  - **Estimated current draw** from calibration matrix (TEST 3 data)
  - **Enclosure temperature** from DS18B20
- **Safety (V2 of main app):**
  - Temp > 70 °C → force fan 100 %
  - Temp > 85 °C → fault state, PWM disabled, clear Serial message

---

## Future: climate auto-mode (ESP-NOW)

A separate ESP-based **remote humidity + temperature node** will talk to the fan controller over **ESP-NOW** (no WiFi router dependency for sensor link).

**Goal:** run the fan as slowly as possible while keeping the electronics enclosure climate acceptable aboard Dorinda.

This is **out of scope** for the initial release but drives architecture choices today:

- Keep fan control logic separate from UI / transport
- Store calibration matrix in a loadable format (SPIFFS or JSON)
- Reserve a `ControlMode` enum: `MANUAL`, `OVERRIDE`, `AUTO` (stubbed until ESP-NOW node exists)

Details: [docs/architecture.md](docs/architecture.md#future-esp-now-climate-node)

---

## Repository layout

```
dorinda-fan-control/
├── README.md                 ← you are here
├── docs/
│   ├── hardware.md           # Wiring, pin map, power notes
│   ├── roadmap.md            # TEST 1–3 + main app acceptance criteria
│   ├── test-log.md           # Bench results & human verification sign-offs
│   ├── architecture.md       # BSP dependency, modules, data flow
│   └── decisions/            # ADR log — why we chose X
├── cad/                      # Mechanical drawings (Autodesk → export here)
├── firmware/
│   ├── platformio.ini        # (planned) PlatformIO project root
│   ├── tests/                # Incremental bring-up programs
│   └── apps/main/            # Production firmware
└── vendor/
    └── bakerbug-firmware/    # git submodule → BSP only
```

---

## Getting started

```bash
git submodule update --init --recursive
cp secrets.env.example secrets.env   # WIFI_MODE=AP, SSID, password

make test1-dev    # TEST 1 — serial sensors (done)
make test2-dev    # TEST 2 — WiFi AP + web (done)
make test3-dev    # TEST 3 — pot→PWM + cal matrix (current)
```

See [firmware/README.md](firmware/README.md) and [docs/test-log.md](docs/test-log.md).

---

## CAD / mechanical

Mechanical design is authored in **Autodesk** and will be exported to [cad/](cad/).

Expected contents once added:

- Enclosure / mounting plate drawings
- Connector placement
- Keep-out zones for 12 V routing vs low-voltage signals

---

## Related projects

| Repo | Role |
|------|------|
| [bakerbug-firmware](https://github.com/BakerStack/bakerbug-firmware) | Shared BSP (WiFi, GPIO, TLS, …) — vendored as submodule |
| *(future)* `dorinda-climate-node` | ESP-NOW remote temp/humidity sensor |

---

## License

Apache 2.0 — see [LICENSE](LICENSE).
