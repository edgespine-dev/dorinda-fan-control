# ADR-0019: HBOM and SBOM for showcase traceability

**Status:** Accepted  
**Date:** 2026-07-06

## Context

Dorinda fan control is a **public showcase** repo. Stakeholders may ask what the product is made of (hardware) and what it runs (software). EU CRA-style thinking favours traceability even before formal conformity work.

Amazon EU short links rot quickly; procurement must survive without them.

## Decision

1. **`docs/hbom.md`** — Hardware Bill of Materials with **procurement-neutral specifications** per line item (voltage, current, interface, Dorinda pin). Reference Amazon links and **archived listing titles** are secondary.
2. **`docs/sbom.md`** — Human-readable software BOM index (first-party modules, submodule pin, key PlatformIO libs). Machine-readable CycloneDX is a **follow-up**.
3. Link HBOM from [hardware.md](../hardware.md) and README quick orientation.

Mapping of user-supplied Amazon short codes (2026-07-06), verified by fetching listings:

| Short link | HBOM-ID | Archived product |
|------------|---------|------------------|
| `01ntzGoL` | HBOM-001 | AZDelivery D1 Mini ESP8266-12F CH340G (ASIN B0754W6Z2F) |
| `05YXt20E` | HBOM-002 | Vegena dual MOSFET 15 A PWM module (ASIN B0C3LXNK7T) |
| `0b5MPv1g` | HBOM-006 | Hailege XL4015 buck 4–38 V → 1.25–36 V 5 A (ASIN B07XRF9NWP) |
| `0fsmNuAP` | HBOM-005 | B50K linear pot (user-confirmed; listing bot-gated) |
| `0aHGjEvU` | HBOM-008 | 10A10 rectifier diodes 30-pack (optional) |

DS18B20, 4.7 kΩ, and 12 V fan: no reference link — procurement-neutral specs only.

## Consequences

**Positive:**

- Anyone can rebuild or audit the physical design without Amazon
- Clear split: HBOM (physical) vs SBOM (firmware)

**Negative:**

- Inferred link→part mapping for three listings may need correction when titles are known
- SBOM not yet auto-generated — manual drift risk

**Follow-ups:**

- User to paste B50K pot ASIN/title from order history (`0fsmNuAP`)
- Add DS18B20 + fan reference links when sourced for boat install
- CycloneDX on release tag; `SECURITY.md`
