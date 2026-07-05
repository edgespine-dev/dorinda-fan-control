# Hardware

Enclosure fan controller for **Dorinda**. All low-voltage logic runs at 3.3 V (ESP8266) or 5 V (D1 Mini regulator input). Fan switching is **12 V low-side** via a MOSFET module.

---

## Block diagram

```
  12 V supply ─────┬──────────────────────────► 12 V fan (+)
                   │
                   ▼
              [ Buck 12→5 V ]
                   │
         5 V ──────┼──────► D1 Mini 5V
         GND ──────┴──────► D1 Mini GND ──┬──► MOSFET module GND
                                          └──► DS18B20 GND, Pot GND

  D1 Mini D5 (PWM) ──────────────────────► MOSFET TRIG/PWM
  D1 Mini D2 ──────── DS18B20 DATA (+ 4.7k to 3V3)
  D1 Mini A0 ──────── Pot wiper
```

---

## Bill of materials (core)

| Item | Notes |
|------|-------|
| Wemos D1 Mini | ESP8266, ESP-12F module |
| XY-MOS dual-MOSFET module | Low-side driver; see [MOSFET module](#mosfet-module-dual-mos-low-side-driver) |
| DS18B20 | Waterproof probe recommended for enclosure |
| 4.7 kΩ resistor | OneWire pull-up, DATA → 3V3 |
| B50K potentiometer | Linear taper, panel mount |
| Buck converter | 12 V → 5 V, adequate headroom for fan + ESP peak current |
| 12 V DC fan | Sized for enclosure volume |
| Fuses / protection | *(document in CAD when mechanical design is added)* |

---

## Pin map (Wemos D1 Mini)

| Function | Arduino pin | ESP GPIO | Direction |
|----------|-------------|----------|-----------|
| Potentiometer wiper | **A0** | ADC0 | Input |
| DS18B20 data | **D2** | GPIO4 | Bidirectional (OneWire) |
| Fan PWM → MOSFET | **D5** | GPIO14 | Output (PWM) |

### PWM note

ESP8266 drives **D5** with `analogWrite()` range **0–1023**. Firmware sets **20 kHz** PWM (`analogWriteFreq(20000)`) — within the MOSFET module’s **0–20 kHz** input spec and above audible whine for most fans.

Pot maps linearly to duty: 0 % → PWM 0, 100 % → PWM 1023 (calibrated ADC range from TEST 1: min=5, max=1024).

### ADC note

A0 accepts **0–3.3 V** max. Pot is wired across 3V3–GND so wiper stays within range.

---

## Power & grounding

**Critical:** single common ground reference between:

1. 12 V supply negative
2. Buck converter GND
3. D1 Mini GND
4. MOSFET module GND

The MOSFET module switches the **low side** of the fan: fan (+) to 12 V+, fan (−) through the module to GND.

Do **not** tie 12 V positive to any ESP pin.

---

## DS18B20 wiring

```
  3V3 ────┬──── VDD (DS18B20)
          │
         4k7
          │
  D2  ────┴──── DATA
  GND ──────── GND
```

Use short leads inside the enclosure. If cable length exceeds ~1 m, consider parasitic power mode (not planned for V1).

---

## Potentiometer wiring

```
  3V3 ─── outer pin
  GND ─── outer pin
  A0  ─── wiper (center)
```

**TEST 1 goal:** log raw ADC min/max with pot at stops. Expect ~0–1023 but mechanical end stops may not reach extremes.

---

## MOSFET module (dual MOS low-side driver)

Typical board: **XY-MOS**-style dual parallel MOSFET module (same class as V921 listings).

| Spec | Value |
|------|-------|
| Load voltage | DC **5–36 V** (Dorinda: **12 V** fan) |
| Continuous current | **15 A** @ 25 °C (with heatsinking) |
| Peak / max rating | up to **30 A** with extra heat sinking |
| Power rating | **400 W** |
| Operating temp | **−40 °C … +85 °C** |
| Control input | **TRIG/PWM** — 3.3 V logic OK (ESP8266 D5) |
| Active level | High = ON, low = OFF |
| PWM frequency | **0–20 kHz** (firmware uses **20 kHz**) |
| Topology | Two MOSFETs in **parallel** — lower R<sub>DS(on)</sub>, higher current |

**Why 20 kHz:** ESP8266 default ~1 kHz PWM can produce audible fan/coils whine. 20 kHz is the top of the module spec and typically inaudible.

### Wiring

```
  D1 Mini GND  ───  Module GND
  D1 Mini D5   ───  Module TRIG/PWM (signal)
  12 V+        ───  Fan +
  Fan −        ───  Module switched output (LOAD+/LOAD− per board silk)
  12 V−        ───  Module GND (common with ESP GND)
```

Verify your specific XY-MOS board silkscreen — some boards label pins `+` / `-` / `S` instead of TRIG/GND.

---

## Mechanical / CAD

Drawings will live in [`../cad/`](../cad/). Include:

- Board mounting standoffs
- Pot and fan cutouts
- Cable glands for 12 V vs signal separation
- DS18B20 probe placement (measure air inside bay, not on hot MOSFET)
