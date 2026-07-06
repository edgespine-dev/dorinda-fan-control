# Hardware Bill of Materials (HBOM)

Procurement-neutral list for the **Dorinda enclosure fan controller** showcase build.  
**Specifications in this file are authoritative.** Amazon short links rot; use the spec columns to source from any vendor.

Firmware bill of materials (libraries, PlatformIO deps): see [SBOM](sbom.md).

---

## How to use this document

| Column | Meaning |
|--------|---------|
| **HBOM-ID** | Stable reference for docs, issues, and compliance packs |
| **Specification** | Minimum technical requirements — match or exceed |
| **Search / substitutes** | Keywords for Mouser, Farnell, AliExpress, local marine chandlers |
| **Dorinda** | Role and pin in this project |
| **Reference procurement** | Link used on the original bench build (may be dead) |
| **Archived listing** | Product title + key specs captured **2026-07-06** from Amazon.se |

Qty for assembly **1** unless noted. Logic at **3.3 V** (ESP8266) or **5 V** (D1 Mini input). Fan switching is **12 V low-side**.

---

## Reference link index (original shopping list)

| Short link | HBOM-ID | Archived product |
|------------|---------|------------------|
| [01ntzGoL](https://amzn.eu/d/01ntzGoL) | HBOM-001 | AZDelivery D1 Mini ESP8266-12F (5-pack) |
| [05YXt20E](https://amzn.eu/d/05YXt20E) | HBOM-002 | Vegena dual MOSFET PWM driver (20-pack) |
| [0b5MPv1g](https://amzn.eu/d/0b5MPv1g) | HBOM-006 | Hailege XL4015 buck module (5-pack) |
| [0fsmNuAP](https://amzn.eu/d/0fsmNuAP) | HBOM-005 | B50K linear pot *(user-confirmed; listing gated)* |
| [0aHGjEvU](https://amzn.eu/d/0aHGjEvU) | HBOM-008 | 10A10 rectifier diodes 30-pack *(optional)* |

DS18B20, 4.7 kΩ resistor, and 12 V fan were **not** on the Amazon list — spec-only rows below.

---

## Core assembly

### HBOM-001 — Microcontroller board

| Field | Value |
|-------|-------|
| **Specification** | **Wemos D1 Mini** form factor (or pin-compatible clone): **ESP8266 ESP-12F**, **802.11 b/g/n**, **3.3 V** GPIO (do not exceed 3.3 V on pins), **5 V** input via onboard regulator (`5V` pin or Micro-USB), **≥4 MB** flash, **CH340** USB-serial (or CP2102 class), **1× ADC** 0–3.3 V (A0), Arduino/PlatformIO. Board size ~**25.6 × 34.2 mm** class |
| **Search** | `Wemos D1 Mini ESP8266`, `LOLIN D1 mini`, `ESP8266-12F CH340`, `AZDelivery D1 Mini` |
| **Dorinda** | Main controller; [pin map](hardware.md#pin-map-wemos-d1-mini) |
| **Reference** | `https://amzn.eu/d/01ntzGoL` |
| **Archived listing** | **AZDelivery 5× D1 Mini NodeMcu** med ESP8266-12F WLAN CH340G Lua, Arduino-kompatibel — **ASIN B0754W6Z2F** *(5-pack; one board per controller)* |

---

### HBOM-002 — Low-side MOSFET driver module

| Field | Value |
|-------|-------|
| **Specification** | **Dual N-channel MOSFET** low-side module: load **DC 5–36 V** (Dorinda **12 V**), **≥15 A** continuous @ room temp, **≤30 A** peak with heatsinking, **≤400 W** module rating. Control **3.3–20 V** logic, **high = ON**, **PWM 0–20 kHz**. Operating temp **−40 … +85 °C**. Common **GND** with MCU |
| **Search** | `dual MOSFET module 15A PWM`, `XY-MOS low side`, `V921 MOSFET driver 3.3V`, `ground source N-MOS switch module` |
| **Dorinda** | **D5** → **TRIG/PWM**; fan (−) through switched output; [wiring](hardware.md#mosfet-module-dual-mos-low-side-driver) |
| **Reference** | `https://amzn.eu/d/05YXt20E` |
| **Archived listing** | **Vegena dual MOSFET driver module** 5–36 V, 15 A (max 30 A), 400 W, PWM 0–20 kHz, trigger 3.3–20 V — **ASIN B0C3LXNK7T** *(listing is 20-pack; one module per controller)* |

---

### HBOM-003 — Enclosure temperature sensor

| Field | Value |
|-------|-------|
| **Specification** | **DS18B20** digital temperature sensor, **OneWire**, **3.0–5.5 V** (use **3.3 V** on ESP8266), **−55 … +125 °C**, **±0.5 °C** typical (0–70 °C). **Waterproof stainless-steel probe** on cable (≥0.5 m) recommended for bay air |
| **Search** | `DS18B20 waterproof probe`, `DS18B20 cable sensor stainless` |
| **Dorinda** | **D2** (GPIO4) + **HBOM-004** pull-up; [wiring](hardware.md#ds18b20-wiring) |
| **Reference** | — |
| **Archived listing** | — *(not on original Amazon list — add link when sourced)* |

---

### HBOM-004 — OneWire pull-up resistor

| Field | Value |
|-------|-------|
| **Specification** | **4.7 kΩ** (4k7), **¼ W** or larger, **≥1 %** tolerance |
| **Search** | `4.7k resistor 0.25W` |
| **Dorinda** | Between DS18B20 **DATA** and **3V3** |
| **Reference** | — |
| **Archived listing** | — |

---

### HBOM-005 — Manual setpoint potentiometer

| Field | Value |
|-------|-------|
| **Specification** | **50 kΩ** linear (**type B / B50K**), **3-terminal** rotary: end–wiper–end. **Panel or PCB mount**, **screw terminals**. Carbon-film element, metal body. Shaft **Ø6 mm × 15 mm** (ridged). Knob **Ø15 mm** (height ~17 mm), bore **Ø6 mm**; nut and washer included. **Not** audio/log taper |
| **Search** | `B50K linear potentiometer 6mm shaft`, `type B 50K panel mount screw terminal` |
| **Dorinda** | **A0** ← wiper; outer pins **3V3** and **GND**; **ADC 5–1024** ([test-log](test-log.md#test-1--sensor-bring-up)) |
| **Reference** | `https://amzn.eu/d/0fsmNuAP` *(unavailable 2026-07-06)* |
| **Archived listing** | **YYWEi** 22-pc kit: *Linjär potentiometer, 3 terminaler, roterande, B5K B10K B20K **B50K** B100K B250K, vridknapp, muttrar och bricka* — use **one B50K** per controller |

---

### HBOM-006 — 12 V → 5 V DC-DC converter

| Field | Value |
|-------|-------|
| **Specification** | Non-isolated **buck** module, **XL4015** class (or equivalent): **input 4–38 V DC** (12 V boat bus), **output 1.25–36 V adjustable** — **set to 5.0 V** before connecting D1 Mini. **Output ≥2 A** continuous (**≤5 A** module rating; vendor recommends **<4.5 A** sustained). Efficiency **~96 %** class, switching **~180 kHz**. Overcurrent / thermal protection. **Common GND** with 12 V return and ESP |
| **Search** | `XL4015 buck 5V`, `12V to 5V buck 5A adjustable`, `LM2596 5V` *(older alternative)* |
| **Dorinda** | **5 V** → D1 Mini `5V`; **GND** → common ground. **Trim pot on module** — verify 5.0 V with meter under WiFi load |
| **Reference** | `https://amzn.eu/d/0b5MPv1g` |
| **Archived listing** | **Hailege 5× XL4015 DC-DC buck** 4–38 V → 1.25–36 V, 5 A (recommended <4.5 A), 96 % — **ASIN B07XRF9NWP** *(5-pack; one module per controller)* |

---

### HBOM-007 — 12 V DC enclosure fan

| Field | Value |
|-------|-------|
| **Specification** | **12 V DC** brushless fan, size per enclosure CAD. Supply and **HBOM-002** must handle **≥6 A** peak (bench full speed **~5.6 A** @ 12 V — [cal matrix](../firmware/data/cal_matrix.json)). **2-wire** (+/−) OK — speed via low-side PWM on negative lead |
| **Search** | `12V DC fan` + diameter from CAD; verify **A** on datasheet |
| **Dorinda** | Fan **+** → **12 V+**; fan **−** → MOSFET module load terminal |
| **Reference** | — |
| **Archived listing** | — *(on-boat fan TBD — document when installed)* |

---

## Protection / fuses

### HBOM-009 — Supply cable fuse (10 A)

| Field | Value |
|-------|-------|
| **Specification** | **10 A** fuse in the **incoming 12 V DC supply cable** to the controller enclosure (inline holder or fuse block). **12 V DC** rated. Protects the **entire** unit — fan, MOSFET, buck, and wiring. Automotive ATO/ATC or marine inline holder acceptable |
| **Search** | `12V 10A inline fuse holder`, `ATO 10A fuse`, `marine DC fuse 10A` |
| **Dorinda** | First fuse after boat 12 V tap; upstream of fan (+) and buck branches — see [power diagram](hardware.md#power--grounding) |
| **Reference** | — |
| **Archived listing** | — *(Dorinda build — user-specified 2026-07-06)* |

---

### HBOM-010 — Buck branch fuse (1 A glass)

| Field | Value |
|-------|-------|
| **Specification** | **1 A** **glass** fuse (**5×20 mm** typical), **quick-blow**, in series on **12 V+ to buck converter only** (HBOM-006). Placed **after** HBOM-009, **before** buck `VIN+`. Protects logic branch if buck or 5 V wiring fails short |
| **Search** | `5x20mm glass fuse 1A`, `fuse holder 5x20` |
| **Dorinda** | `12 V+` → **1 A glass** → buck → 5 V → D1 Mini |
| **Reference** | — |
| **Archived listing** | — *(Dorinda build — user-specified 2026-07-06)* |

---

## Optional parts

### HBOM-008 — Rectifier diodes (optional)

| Field | Value |
|-------|-------|
| **Specification** | Axial diode **10A10** class: **10 A** forward, **1000 V** reverse (rectifier / high-voltage barrier diode). For discrete PSU or protection experiments — **not wired** in default Dorinda firmware |
| **Search** | `10A10 diode`, `10A 1000V rectifier diode` |
| **Dorinda** | Optional in 12 V input protection — not in pin map |
| **Reference** | `https://amzn.eu/d/0aHGjEvU` |
| **Archived listing** | **30× rectifier diodes 10A10** 1000 V / 10 A (Wjelet / JUNZHIJIKEJI listing, Amazon.se 2026-07-06) |

---

## Summary table

| HBOM-ID | Component | Qty used | Critical spec | Ref link | ASIN (archive) |
|---------|-----------|----------|---------------|----------|----------------|
| HBOM-001 | D1 Mini ESP8266 | 1 | ESP-12F, CH340, 3.3 V IO | [01ntzGoL](https://amzn.eu/d/01ntzGoL) | B0754W6Z2F |
| HBOM-002 | Dual MOSFET driver | 1 | 12 V, 15 A, PWM 0–20 kHz | [05YXt20E](https://amzn.eu/d/05YXt20E) | B0C3LXNK7T |
| HBOM-003 | DS18B20 probe | 1 | OneWire, 3.3 V, waterproof | — | — |
| HBOM-004 | Resistor 4.7 kΩ | 1 | ¼ W pull-up | — | — |
| HBOM-005 | Pot B50K linear | 1 | Type B, 6 mm shaft, screw terminals | *(dead)* | — |
| HBOM-006 | Buck XL4015 | 1 | 12 V → **5 V** trimmed, ≥2 A | [0b5MPv1g](https://amzn.eu/d/0b5MPv1g) | B07XRF9NWP |
| HBOM-007 | Fan 12 V DC | 1 | ≤6 A bench peak | — | — |
| HBOM-008 | Diode 10A10 (opt.) | 1* | 10 A / 1000 V | [0aHGjEvU](https://amzn.eu/d/0aHGjEvU) | — |
| HBOM-009 | Fuse supply **10 A** | 1 | Inline on 12 V cable | — | — |
| HBOM-010 | Fuse buck **1 A** glass | 1 | 5×20 mm, 12 V+ → buck | — | — |

\*HBOM-008 listing sold as 30-piece kit.

---

## Compliance note (showcase)

HBOM answers *what physical product is this?* for public reference builds. It does **not** replace software SBOM, `SECURITY.md`, or CE technical files. See [ADR-0019](decisions/0019-hbom-sbom-showcase.md).

When a link dies: update **Archived listing** / ASIN from order history — keep **Specification** unless the design changes.
