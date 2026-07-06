# Software Bill of Materials (SBOM) — overview

Firmware **v1.0** for Dorinda fan controller. Full machine-readable SBOM generation is **planned** (CycloneDX from PlatformIO lockfile). This page is the human-readable index for showcase traceability.

**Release process:** [RELEASE.md](RELEASE.md) — git tags, submodule pin, on-device SHA via `/api/version`.

**Hardware parts:** [HBOM](hbom.md)

---

## Product identity

| Field | Value |
|-------|-------|
| Product | Dorinda fan control — main app |
| Repository | `dorinda-fan-control` |
| Release tag | `v1.0` |
| MCU | ESP8266 (Wemos D1 Mini class) |
| BSP submodule | `bakerbug-firmware@d64c2a0` (pinned at `v1.0`) |

---

## First-party firmware modules

| Component | Path | Role |
|-----------|------|------|
| Main application | `firmware/apps/main/` | REST API, web UI, loop scheduling |
| Control policy | `firmware/lib/control/` | Manual / remote, boot arm |
| Calibration matrix | `firmware/lib/cal_matrix/` | PWM → mA (bench_lab) |
| WiFi boot | `firmware/lib/dfc_wifi/` | AP/STA from secrets |
| Pin map | `firmware/lib/hw/pins.h` | Dorinda wiring |

---

## bakerbug-firmware (submodule)

| Component | Path | Role |
|-----------|------|------|
| BSP ESP8266 | `firmware/bsp/esp8266/` | WiFi, log |
| Drivers | `firmware/lib/drivers/` | DS18B20, potentiometer, fan_control |

---

## Key third-party libraries (PlatformIO)

Resolved at build time via `[env:main]` in `firmware/platformio.ini`. Pin versions in release notes when tagging.

| Library | Purpose |
|---------|---------|
| `framework-arduinoespressif8266` | Arduino core for ESP8266 |
| `ESPAsyncWebServer-esphome` | HTTP server |
| `ArduinoJson` | REST JSON bodies |
| `OneWire` / `DallasTemperature` | DS18B20 (via drivers) |
| `ESP8266WiFi` | WiFi stack |

Regenerate dependency list locally:

```bash
cd firmware && DFC_SRC=apps/main/src pio run -e main -v 2>&1 | rg 'Library|Found'
```

---

## Planned

- [ ] CycloneDX JSON artifact attached to GitHub Releases
- [ ] `SECURITY.md` with vulnerability reporting contact
- [ ] Pin exact `lib_deps` versions in `platformio.ini` for reproducible SBOM
