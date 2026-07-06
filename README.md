# Dorinda Fan Control

Smart 12 V enclosure fan controller for **Dorinda** (sailboat electronics bay). Built on **ESP8266**, with incremental bring-up tests, a calibration workflow for PWM→current mapping, and a production web UI for manual control with safety overrides.

> **If you are reading this after a long break:** start with [Quick orientation](#quick-orientation). **v1.0 shipped** (2026-07-06) — main app with REST, pot, bench cal, presets. Next: **Safety V2** (thermal) · **on-boat install** · **ESP-NOW** climate node.

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
| **`hw/` driver API policy** | [docs/architecture.md#hardware-layer-hw](docs/architecture.md#hardware-layer-hw) · [ADR-0012](docs/decisions/0012-hw-component-api.md) |
| Test phases & main app spec | [docs/roadmap.md](docs/roadmap.md) |
| **Design decision log (ADR)** | [docs/decisions/](docs/decisions/) — [0014](docs/decisions/0014-cooperative-loop-timing.md) timing · [0015](docs/decisions/0015-boot-safe-fan-off.md) boot safety · [0018](docs/decisions/0018-fan-quick-presets.md) presets |
| Firmware | [firmware/](firmware/) — `make main-dev` |

**Build philosophy:** incremental TEST programs, then production main app. **v1.0** runs cooperative 1 s / 5 s ticks — not a hard real-time system ([ADR-0014](docs/decisions/0014-cooperative-loop-timing.md)).

**External dependency:** this repo consumes the **Board Support Package (BSP)** from [bakerbug-firmware](https://github.com/BakerStack/bakerbug-firmware) for WiFi and other platform abstractions — not MQTT or cloud stack.

---

## What this project does

```
┌─────────────────────────────────────────────────────────────┐
│  Operator (browser on phone/laptop)                         │
│  • Pot, temp, estimated current                           │
│  • Slider remote + OFF / Silent / FULL presets              │
│  • Remote clears when physical pot moves (>4 %)             │
└──────────────────────────┬──────────────────────────────────┘
                           │ HTTP (WiFi)
┌──────────────────────────▼──────────────────────────────────┐
│  Wemos D1 Mini (ESP8266)                                    │
│  • Read B50K pot (A0)                                       │
│  • Read DS18B20 enclosure temp (D2)                         │
│  • PWM fan via MOSFET module (D5)                           │
│  • Lookup PWM → current from bench cal matrix               │
│  • Fan OFF at boot until pot moves ([ADR-0015](docs/decisions/0015-boot-safe-fan-off.md)) │
└──────────────────────────┬──────────────────────────────────┘
                           │ 12 V low-side switch
┌──────────────────────────▼──────────────────────────────────┐
│  12 V fan + XY-MOS MOSFET module                            │
└─────────────────────────────────────────────────────────────┘

Future: forepeak ESP32 ──HTTP POST /api/climate──► remote fan setpoint
(planned ESP-NOW link — [ADR-0009](docs/decisions/0009-esp-now-future-climate-node.md))
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
| **TEST 3** | `firmware/tests/test3_pwm_matrix/` | Pot→PWM + cal matrix (regression) | ✅ Bench cal in repo |
| **Main app** | `firmware/apps/main/` | Production REST controller | ✅ **v1.0** |

Detailed requirements for each phase: [docs/roadmap.md](docs/roadmap.md)

---

## Main application (v1.0)

Tag **`v1.0`** — see [docs/roadmap.md](docs/roadmap.md) and [docs/architecture.md](docs/architecture.md).

- **WiFi** — AP default (`dfan`); STA optional via `secrets.env` ([ADR-0017](docs/decisions/0017-dfc-wifi-shared-boot.md))
- **Web** — dashboard with pot, temp, est. mA, remote slider
- **Presets** — OFF · Silent (~1 V / 8 %) · FULL ([ADR-0018](docs/decisions/0018-fan-quick-presets.md))
- **Remote** — `PUT /api/fan/setpoint`; pot move >4 % clears remote ([ADR-0007](docs/decisions/0007-override-drop-on-pot-move.md))
- **Forepeak** — `POST /api/climate` with optional `fan_percent`
- **Boot safety** — PWM off until pot nudged **or** remote command ([ADR-0015](docs/decisions/0015-boot-safe-fan-off.md))
- **Timing** — 1 s control, 5 s temp ([ADR-0014](docs/decisions/0014-cooperative-loop-timing.md))

**Not in v1.0 (planned V2):** thermal fault at 70 °C / 85 °C ([ADR-0008](docs/decisions/0008-safety-thresholds-v2.md)).

```bash
make main-dev    # from repo root — upload + serial monitor
```

---

## Future: climate auto-mode (ESP-NOW)

A separate ESP-based **remote humidity + temperature node** will talk to the fan controller over **ESP-NOW** (no WiFi router dependency for sensor link).

**Goal:** run the fan as slowly as possible while keeping the electronics enclosure climate acceptable aboard Dorinda.

This is **out of scope** for the initial release but drives architecture choices today:

- Keep fan control in `control/` lib separate from HTTP
- Bench matrix in repo + embedded on main ([ADR-0016](docs/decisions/0016-embedded-cal-matrix-main.md))
- `control::Mode`: `Manual`, `Remote` — `Auto` when ESP-NOW node exists

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
│   ├── platformio.ini
│   ├── lib/
│   │   ├── hw/               # pins.h only
│   │   ├── control/          # manual / remote policy
│   │   ├── cal_matrix/       # PWM → mA
│   │   └── dfc_wifi/         # AP/STA boot
│   ├── apps/main/            # Production firmware (v1.0)
│   ├── tests/                # TEST 1–3 bring-up
│   └── data/cal_matrix.json  # Bench cal (canonical)
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
make test3-dev    # TEST 3 — cal regression (LittleFS)
make main-dev     # Main app v1.0 — REST + pot + presets
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
