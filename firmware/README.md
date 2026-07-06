# Firmware

PlatformIO project for Dorinda fan controller. **Release:** main app `v1.0`.

## Layout

```
firmware/
├── platformio.ini
├── lib/
│   ├── hw/                    # Dorinda pin map (`hw/pins.h`)
│   ├── control/               # Manual / remote policy, boot arm
│   ├── cal_matrix/            # PWM → mA (embedded + TEST 3 LittleFS)
│   └── dfc_wifi/              # AP/STA boot from secrets.env
├── apps/main/                 # Production REST app (v1.0)
├── tests/
│   ├── test1_sensors/
│   ├── test2_wifi_pot/
│   └── test3_pwm_matrix/
└── data/
    └── cal_matrix.json        # Canonical bench cal (bench_lab)
```

**Drivers:** `vendor/bakerbug-firmware/firmware/lib/drivers/` — pot, DS18B20, fan_control.  
**BSP:** WiFi, log — linked from TEST 2 onward.

Design: [docs/architecture.md](../docs/architecture.md) · ADRs [0014](../docs/decisions/0014-cooperative-loop-timing.md)–[0018](../docs/decisions/0018-fan-quick-presets.md).

## WiFi credentials

```bash
cp secrets.env.example secrets.env   # WIFI_MODE=AP, SSID, password (≥8 chars)
```

Generated at build: `firmware/include/dfc_secrets.h`.

## TEST 1 — sensors (serial)

```bash
make test1-dev
```

No WiFi, no fan PWM — safe without 12 V.

## TEST 2 — WiFi + pot web

```bash
make test2-dev
```

1. Join AP from `secrets.env` (default SSID in secrets)
2. `http://192.168.4.1/` — pot, temp; `/api/status`

Uses `dfc_wifi` + BSP softAP ([ADR-0017](../docs/decisions/0017-dfc-wifi-shared-boot.md)).

## TEST 3 — PWM + cal matrix (regression)

**Requires 12 V + fan.** LittleFS + `/cal` editor. Bench matrix already in repo — use TEST 3 only to re-measure.

```bash
make test3-dev
```

## Main app — REST + pot (v1.0)

```bash
# from repo root (not firmware/)
make main-dev
```

### Behaviour

| Area | Detail |
|------|--------|
| **WiFi** | AP default; `dfc_wifi_boot()` |
| **Timing** | 1 s pot/fan · 5 s temp · `yield()` for radio ([ADR-0014](../docs/decisions/0014-cooperative-loop-timing.md)) |
| **Boot** | Fan off until pot >4 % **or** remote setpoint ([ADR-0015](../docs/decisions/0015-boot-safe-fan-off.md)) |
| **Cal** | Embedded bench matrix, no LittleFS ([ADR-0016](../docs/decisions/0016-embedded-cal-matrix-main.md)) |
| **Manual** | Pot → ramped PWM @ 18 kHz |
| **Remote** | Slider or API; pot clears remote (>4 % deadband) |

### Web UI

- Live pot %, fan %, bay temp, est. mA
- Slider + **Apply remote** / **Back to pot**
- **OFF · Silent · FULL** — [ADR-0018](../docs/decisions/0018-fan-quick-presets.md)

### REST API

| Method | Path | Notes |
|--------|------|-------|
| GET | `/api/status` | Full state |
| GET | `/api/presets` | `{"off":0,"silent":8,"full":100}` |
| PUT | `/api/fan/setpoint` | `{"percent":0-100}` |
| DELETE | `/api/fan/remote` | Back to pot |
| POST | `/api/climate` | Forepeak node |

Forepeak example:

```bash
curl -X POST http://192.168.4.1/api/climate \
  -H 'Content-Type: application/json' \
  -d '{"temp_c":22,"humidity_pct":55,"co2_ppm":800,"fan_percent":42}'
```

Full reference: [docs/architecture.md](../docs/architecture.md#rest-api-main-app).

### Serial

```
AP manual ip=192.168.4.1 cli=0 Pot=0 (0%) Fan=0% PWM=0 Temp=23.5C Est=0 mA [BOOT OFF]
```

## Build notes

- `make sync-bsp-libdeps` — clears stale BSP cache before test2/test3/main builds
- `make main-erase` — full chip erase if flash layout was experimented with

## Submodule

```bash
git submodule update --init --recursive
```

Pin submodule commit when tagging releases.
