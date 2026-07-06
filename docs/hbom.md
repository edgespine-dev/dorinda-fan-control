# Hardware Bill of Materials (HBOM)

Procurement-neutral list for the **Dorinda enclosure fan controller** showcase build.  
**Specifications in this file are authoritative.** Amazon short links rot; use the spec columns to source from any vendor.

Firmware bill of materials (libraries, PlatformIO deps): see [SBOM](sbom.md) *(planned)*.

---

## How to use this document

| Column | Meaning |
|--------|---------|
| **HBOM-ID** | Stable reference for docs, issues, and compliance packs |
| **Specification** | Minimum technical requirements — match or exceed |
| **Search / substitutes** | Keywords for Mouser, Farnell, AliExpress, local marine chandlers |
| **Dorinda** | Role and pin in this project |
| **Reference procurement** | Link used on the original bench build (may be dead) |
| **Archived listing** | Product title captured when link was added (2026-07-06) |

Qty **1** unless noted. All logic runs at **3.3 V** (ESP8266) or **5 V** (D1 Mini input). Fan switching is **12 V low-side**.

---

## Core assembly

### HBOM-001 — Microcontroller board

| Field | Value |
|-------|-------|
| **Specification** | ESP8266 module board, **Wemos D1 Mini** class (or pin-compatible clone): ESP-12F, **3.3 V** GPIO, **5 V** input via onboard regulator (Micro-USB or `5V` pin), **≥4 MB** flash, Arduino/PlatformIO support, **80 MHz** class |
| **Search** | `Wemos D1 Mini ESP8266`, `LOLIN D1 mini`, `ESP8266MOD 4MB` |
| **Dorinda** | Main controller; see [pin map](hardware.md#pin-map-wemos-d1-mini) |
| **Reference** | `https://amzn.eu/d/01ntzGoL` |
| **Archived listing** | *(title not captured — match specification above)* |

---

### HBOM-002 — Low-side MOSFET driver module

| Field | Value |
|-------|-------|
| **Specification** | **Dual N-channel MOSFET** low-side switch module (XY-MOS / V921 class): load **5–36 V DC** (use **12 V**), **≥15 A** continuous with heatsinking, logic input **3.3–20 V**, **PWM 0–20 kHz**, **high = ON**. On-board gate driver; **GND** common with MCU |
| **Search** | `XY-MOS MOSFET module`, `dual MOS low side 15A PWM`, `V921 MOSFET driver 3.3V` |
| **Dorinda** | **D5** → module **TRIG/PWM**; fan (−) through module; see [MOSFET section](hardware.md#mosfet-module-dual-mos-low-side-driver) |
| **Reference** | `https://amzn.eu/d/0b5MPv1g` |
| **Archived listing** | *(title not captured — match specification above)* |

---

### HBOM-003 — Enclosure temperature sensor

| Field | Value |
|-------|-------|
| **Specification** | **DS18B20** digital temperature sensor, **OneWire**, **3.0–5.5 V**, **−55 … +125 °C**, **±0.5 °C** typical (0–70 °C). **Waterproof stainless probe** variant recommended for bay air measurement |
| **Search** | `DS18B20 waterproof probe`, `DS18B20 cable sensor 1m` |
| **Dorinda** | **D2** (GPIO4) + **4.7 kΩ** pull-up DATA→3V3; see [DS18B20 wiring](hardware.md#ds18b20-wiring) |
| **Reference** | `https://amzn.eu/d/05YXt20E` |
| **Archived listing** | *(title not captured — match specification above)* |

---

### HBOM-004 — OneWire pull-up resistor

| Field | Value |
|-------|-------|
| **Specification** | **4.7 kΩ** (4k7), **¼ W** metal film or thicker, **≥1 %** tolerance |
| **Search** | `4.7k resistor 0.25W` |
| **Dorinda** | Between **DATA** and **3V3** on DS18B20 bus |
| **Reference** | — (generic part, any electronics supplier) |
| **Archived listing** | — |

---

### HBOM-005 — Manual setpoint potentiometer

| Field | Value |
|-------|-------|
| **Specification** | **50 kΩ** linear rotary potentiometer (**B50K**), **3-terminal** (wiper + two ends), panel-mount or shaft knob suitable for helm mounting. Taper: **linear** (not log/audio) |
| **Search** | `B50K potentiometer linear`, `50k ohm rotary pot panel mount` |
| **Dorinda** | **A0** ← wiper; outer pins **3V3** and **GND**; calibrated span **ADC 5–1024** ([test-log](test-log.md#test-1--sensor-bring-up)) |
| **Reference** | `https://amzn.eu/d/0fsmNuAP` |
| **Archived listing** | **B50K** linear potentiometer *(user-confirmed mapping)* |

---

### HBOM-006 — 12 V → 5 V DC-DC converter

| Field | Value |
|-------|-------|
| **Specification** | Step-down (buck) module: **input 8–24 V DC** (nominal **12 V** boat bus), **output 5 V DC**, **≥2 A** continuous (ESP8266 WiFi peaks + margin). Non-isolated OK if **common GND** with load. Efficiency **≥85 %** typical |
| **Search** | `12V to 5V buck 3A`, `LM2596 5V module`, `Mini560 5V buck` |
| **Dorinda** | **5 V** → D1 Mini `5V`; **GND** → common ground plane |
| **Reference** | — |
| **Archived listing** | — |

---

### HBOM-007 — 12 V DC enclosure fan

| Field | Value |
|-------|-------|
| **Specification** | **12 V DC** brushless fan sized for electronics bay volume. **≥6 A** peak capability on supply/MOSFET module (bench full speed **~5.6 A** @ 12 V — [cal matrix](../firmware/data/cal_matrix.json)). **2-wire** (±) sufficient — speed controlled by **low-side PWM** on negative lead |
| **Search** | `12V DC fan 120mm` / size per enclosure CAD; check amp draw on datasheet |
| **Dorinda** | Fan **+** → **12 V+**; fan **−** → MOSFET module switched output |
| **Reference** | — |
| **Archived listing** | — |

---

## Optional / protection (reference link on original BOM)

### HBOM-008 — High-voltage rectifier diodes (optional)

| Field | Value |
|-------|-------|
| **Specification** | Axial rectifier diode **10A10** class: **10 A** forward, **1000 V** reverse, suitable for **DIY power-supply / AC rectifier / protection** projects. Use only if your 12 V feed design calls for discrete rectification or flyback — **not required** for minimal Dorinda low-side PWM build |
| **Search** | `10A10 diode`, `10A 1000V rectifier diode axial` |
| **Dorinda** | Not in default firmware pin map — document if added to 12 V input protection |
| **Reference** | `https://amzn.eu/d/0aHGjEvU` |
| **Archived listing** | *30× Rectifier Diodes 10A10 High Voltage Schottky/rectifier diode 1000 V 10 A (Amazon.se listing, 2026-07-06)* |

---

## Summary table

| HBOM-ID | Component | Qty | Critical spec (short) | Ref link |
|---------|-----------|-----|------------------------|----------|
| HBOM-001 | ESP8266 D1 Mini | 1 | ESP-12F, 4 MB flash, 3.3 V GPIO | [01ntzGoL](https://amzn.eu/d/01ntzGoL) |
| HBOM-002 | XY-MOS low-side driver | 1 | 12 V, ≥15 A, 3.3 V PWM, 0–20 kHz | [0b5MPv1g](https://amzn.eu/d/0b5MPv1g) |
| HBOM-003 | DS18B20 probe | 1 | OneWire, waterproof probe | [05YXt20E](https://amzn.eu/d/05YXt20E) |
| HBOM-004 | Resistor 4.7 kΩ | 1 | ¼ W, pull-up | — |
| HBOM-005 | Potentiometer B50K | 1 | 50 kΩ linear | [0fsmNuAP](https://amzn.eu/d/0fsmNuAP) |
| HBOM-006 | Buck 12→5 V | 1 | ≥2 A @ 5 V | — |
| HBOM-007 | Fan 12 V DC | 1 | sized for bay; ≤6 A bench peak | — |
| HBOM-008 | Diode 10A10 (opt.) | 30* | 10 A / 1000 V | [0aHGjEvU](https://amzn.eu/d/0aHGjEvU) |

\*Kit quantity from archived Amazon listing; adjust for your design.

---

## Compliance note (showcase)

This HBOM supports **design traceability** for a public reference build (CRA-style documentation: *what physical product is this?*). It does **not** replace a software SBOM, vulnerability process, or CE technical file. See [architecture](architecture.md) and [decisions/](decisions/).

When a reference link dies, open a PR updating **Archived listing** only if you have a new URL — **do not** change Specification without remeasuring on bench.
