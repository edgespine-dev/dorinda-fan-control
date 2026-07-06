# Disclaimer

**This document is not legal advice.** It summarises how the authors intend this
open-source project to be used. For binding terms, see [LICENSE](../LICENSE)
(Apache License 2.0).

---

## Software — no warranty

The firmware and documentation are provided **“AS IS”**, without warranty of any kind,
as stated in the Apache 2.0 license. Authors and contributors are **not liable** for
damages arising from use, misuse, or inability to use the software, except where
law forbids such limitation.

---

## Not safety-critical or certified equipment

Dorinda fan control is a **hobby / proof-of-concept / showcase** project:

- It is **not** classified or certified as **marine**, **electrical**, or **safety**
  equipment.
- It is **not** intended for functions that could directly endanger life, vessel
  control, navigation, propulsion, bilge pumping, gas detection, or similar
  **safety-critical** roles.
- It controls **enclosure ventilation** for an electronics bay only.

**Do not rely on this device as your sole protection against overheating, fire, or
equipment failure.** Use appropriate fuses, wiring, and professional judgment.

Thermal auto-shutdown thresholds (70 °C / 85 °C) are **planned for a future release**
([ADR-0008](decisions/0008-safety-thresholds-v2.md)). Until that version is installed
and verified on your hardware, treat V1 as **bench / development** use on a boat.

---

## Electrical and fire risk — your responsibility

You build and wire the hardware **at your own risk**:

- **12 V DC** circuits can cause **fire, burns, or equipment damage** if fused,
  wired, or loaded incorrectly.
- Follow the [hardware guide](hardware.md) and [HBOM](hbom.md); use the specified
  fuse ratings (supply and buck branch).
- MOSFET modules, fans, and buck converters vary by supplier — verify ratings on
  **your** parts before energising.
- The authors do not inspect your installation and do not guarantee compliance with
  local electrical or marine regulations.

---

## Maritime / boat installation

Installation on a vessel is **entirely the operator’s responsibility**:

- Verify suitability for your environment (vibration, moisture, corrosion, EMI).
- This project does not replace manufacturer instructions for commercial off-the-shelf
  components (fan, PSU, MCU module).
- Insurers, flag states, and port authorities may impose requirements this project
  does not meet.

---

## Security

V1 firmware uses an **open WiFi access point** and **unencrypted HTTP** by design
([RELEASE.md](RELEASE.md), [ADR-0020](decisions/0020-v1-security-posture.md)):

- Anyone in radio range may **read or change** fan settings.
- Firmware images are **not** cryptographically signed on the device.
- **Do not** expose secrets, personal data, or safety systems through this interface.

---

## Third-party software and hardware

This project links **third-party libraries** (Arduino core, AsyncWebServer, ArduinoJson,
OneWire, DallasTemperature, etc.) and documents **third-party hardware** in the HBOM.
Each component remains under **its own license**; your obligations when redistributing
or shipping binaries may include **LGPL/MIT attribution and source availability**
for those components. See [sbom.md](sbom.md); a full third-party notice file may be
added later.

The **bakerbug-firmware** submodule is licensed separately (Apache 2.0).

---

## Trademarks

**Dorinda**, **BakerStack**, **EdgeSpine**, product names in the HBOM, and vendor
names are used for **identification only**. This project is not endorsed by any
manufacturer or employer unless explicitly stated elsewhere.

---

## Summary (plain language)

| Topic | Position |
|-------|----------|
| Will it work on my boat? | Maybe — **you** must test and decide |
| Is it approved marine gear? | **No** |
| Can I sue if it breaks something? | Apache 2.0 limits author liability; **consult a lawyer** for your jurisdiction |
| Is the WiFi secure? | **No** — PoC only |
| Who is responsible? | **You**, the builder and operator |

If any of this is unacceptable for your use case, **do not install** this firmware on
a live vessel.
