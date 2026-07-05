# ADR-0006: PWM→current via measured calibration matrix

**Status:** Accepted  
**Date:** 2026-07-05

## Context

The web UI should show **estimated fan current** for a given PWM output. Fan I/V curve is non-linear and depends on the specific fan, MOSFET module, and 12 V supply. A formula (e.g. `I = k × PWM`) would be inaccurate.

## Decision

Build a **calibration matrix** in **TEST 3**: operator sets PWM steps, measures current with a meter, enters values in a web form. Firmware stores `{pwm, current_ma}` knots and uses **linear interpolation** at runtime. Matrix is persisted (SPIFFS/LittleFS/NVS) and dumped to Serial on every edit.

## Consequences

**Positive:**

- Accurate enough for operator awareness (not billing-grade metrology)
- Per-installation calibration handles component spread
- Matrix is data — updatable without reflash

**Negative:**

- Manual bench session required once per hardware build
- Interpolation between sparse points is approximate

**Follow-ups:**

- Document suggested PWM knot set in [roadmap.md](../roadmap.md)
