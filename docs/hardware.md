# Hardware

Enclosure fan controller for **Dorinda**. All low-voltage logic runs at 3.3 V (ESP8266) or 5 V (D1 Mini regulator input). Fan switching is **12 V low-side** via a MOSFET module.

---

## Block diagram

```
  Boat 12V+ ──[10 A fuse, supply cable]──┬──────────────────► 12 V fan (+)
                                          │
                                          ├──[1 A glass fuse]──► Buck 12→5 V
                                          │
  Boat 12V− ─────────────────────────────┴──► common GND
                                                   │
         5 V ──────────────────────────────────────┼──► D1 Mini 5V
         GND ──────────────────────────────────────┴──► D1 Mini GND ──┬──► MOSFET GND
                                                                      └──► DS18B20 GND, Pot GND

  D1 Mini D5 (PWM) ──────────────────────► MOSFET TRIG/PWM
  D1 Mini D2 ──────── DS18B20 DATA (+ 4.7k to 3V3)
  D1 Mini A0 ──────── Pot wiper
```

---

## Bill of materials (core)

Full **HBOM** with procurement-neutral specs and reference Amazon links: **[docs/hbom.md](hbom.md)**.  
Software components: **[docs/sbom.md](sbom.md)**.

| Item | Notes |
|------|-------|
| Wemos D1 Mini | ESP8266, ESP-12F module — [HBOM-001](hbom.md#hbom-001--microcontroller-board) |
| XY-MOS dual-MOSFET module | Low-side driver — [HBOM-002](hbom.md#hbom-002--low-side-mosfet-driver-module) |
| DS18B20 | Waterproof probe recommended — [HBOM-003](hbom.md#hbom-003--enclosure-temperature-sensor) |
| 4.7 kΩ resistor | OneWire pull-up — [HBOM-004](hbom.md#hbom-004--onewire-pull-up-resistor) |
| B50K potentiometer | Linear type **B**, panel mount — [HBOM-005](hbom.md#hbom-005--manual-setpoint-potentiometer) |
| Buck converter | 12 V → 5 V, ≥2 A — [HBOM-006](hbom.md#hbom-006--12-v--5-v-dc-dc-converter) |
| 12 V DC fan | Sized for enclosure — [HBOM-007](hbom.md#hbom-007--12-v-dc-enclosure-fan) |
| Fuse **10 A** | Inline on **supply cable** to whole unit — [HBOM-009](hbom.md#hbom-009--supply-cable-fuse-10-a) |
| Fuse **1 A** glass | **12 V+ → buck** branch only — [HBOM-010](hbom.md#hbom-010--buck-branch-fuse-1-a-glass) |

---

## Pin map (Wemos D1 Mini)

| Function | Arduino pin | ESP GPIO | Direction |
|----------|-------------|----------|-----------|
| Potentiometer wiper | **A0** | ADC0 | Input |
| DS18B20 data | **D2** | GPIO4 | Bidirectional (OneWire) |
| Fan PWM → MOSFET | **D5** | GPIO14 | Output (PWM) |

### PWM note

ESP8266 drives **D5** with `analogWrite()` range **0–1023**. Firmware sets **18 kHz** PWM (`analogWriteFreq(18000)`) — margin under the module’s **0–20 kHz** max, usually above audible whine.

Pot maps linearly to duty via `drivers::pot_read_value_percentage()` (calibrated with `pot_init(pin, 5, 1024)` from TEST 1).

**Ramp (inrush limit):** `drivers::fan_control_set_new_setpoint()` sets the **target** (0–100 %); `drivers::fan_control_loop()` (every `loop()`) steps `fan_control_get_current_value()` by at most **6 % every 200 ms** until the setpoint is reached. Full 0→100 % takes ~3.4 s.

### ADC note

A0 accepts **0–3.3 V** max. Pot is wired across 3V3–GND so wiper stays within range.

---

## Power & grounding

### Fuses

| Fuse | Location | Protects |
|------|----------|----------|
| **10 A** | In **supply cable** from boat 12 V to the controller enclosure | Entire assembly — fan, MOSFET module, buck, and wiring |
| **1 A glass** | In series on **12 V+** feed to **buck converter only** (before HBOM-006) | Logic branch (ESP + sensors). A buck fault does not rely on the 10 A fuse alone |

Use **12 V DC** rated holders. The 1 A glass fuse is typically **5×20 mm** quick-blow on the low-current buck input. Size the 10 A fuse for the fan’s bench peak (~6 A) with margin — see [HBOM-007](hbom.md#hbom-007--12-v-dc-enclosure-fan).

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

**Part:** **B50K** linear (type B), 3-terminal rotary — [HBOM-005](hbom.md#hbom-005--manual-setpoint-potentiometer).  
**Mechanical (YYWEi-class):** shaft **Ø6 mm × 15 mm**; panel mount with nut/washer; knob **Ø15 mm** (6 mm bore).

```
  3V3 ─── outer pin
  GND ─── outer pin
  A0  ─── wiper (center)
```

**TEST 1 goal:** log raw ADC min/max with pot at stops. Expect ~0–1023 but mechanical end stops may not reach extremes.

---

## MOSFET module (dual MOS low-side driver)

Typical board: **XY-MOS**-style **dual parallel MOSFET** switch module (V921-class listings). Low-side (**ground-source**) switching — TRIG high turns load on, TRIG low turns load off.

### Electrical ratings

| Spec | Value |
|------|-------|
| Load voltage | DC **5–36 V** (Dorinda: **12 V** fan) |
| Continuous current | **15 A** @ room temperature (adequate heatsinking) |
| Peak / max current | up to **30 A** with additional heat sinking |
| Power rating | **400 W** |
| Operating temperature | **−40 °C … +85 °C** |

### Control input (TRIG / PWM)

| Spec | Value |
|------|-------|
| Logic | Digital **high = ON**, **low = OFF** |
| Compatible levels | **3.3 V – 20 V** (ESP8266 D5 @ 3.3 V OK) |
| PWM frequency | **0 – 20 kHz** (firmware: **18 kHz**) |
| Interfaces | MCU GPIO, Arduino-class boards, PLC inputs, DC control |

### Design (vendor summary)

- **Dual MOSFETs in parallel** on one driver IC — lower effective R<sub>DS(on)</sub>, higher current capacity, better heat spreading than a single FET.
- On-board **FET driver circuit** — signal conditioning for PWM; intended for ground-referenced (low-side) loads.
- Claimed traits: low distortion, good linearity/crosstalk behaviour, high bandwidth, low drive noise — adequate for PWM fan speed control (not audio-grade, but suitable for switching loads).

### Dorinda usage

| Aspect | Choice |
|--------|--------|
| Load | 12 V DC enclosure fan (low-side switched) |
| ESP pin | **D5** → module **TRIG/PWM** |
| PWM | 18 kHz, 10-bit (0–1023), **ramped** 64 counts / 200 ms (see [PWM note](#pwm-note)) |
| Headroom | Fan << 15 A continuous — large margin on this module |

**Why 18 kHz:** ESP8266 default ~1 kHz PWM causes audible whine. 18 kHz stays clearly below the module’s 20 kHz ceiling (timing/component spread) while still inaudible to most ears.

**Inrush:** Firmware never steps PWM instantly; ramp limits start-up current spikes on fan and module.

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
