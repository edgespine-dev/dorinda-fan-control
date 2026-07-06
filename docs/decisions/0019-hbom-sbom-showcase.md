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

Mapping of user-supplied Amazon short codes (2026-07-06):

| Short link | HBOM-ID | Notes |
|------------|---------|-------|
| `01ntzGoL` | HBOM-001 | D1 Mini — title not archived |
| `0b5MPv1g` | HBOM-002 | MOSFET module — title not archived |
| `05YXt20E` | HBOM-003 | DS18B20 — title not archived |
| `0fsmNuAP` | HBOM-005 | B50K pot — user confirmed |
| `0aHGjEvU` | HBOM-008 | 10A10 diodes — optional; title archived |

Buck converter and 12 V fan have no reference link — spec-only rows.

## Consequences

**Positive:**

- Anyone can rebuild or audit the physical design without Amazon
- Clear split: HBOM (physical) vs SBOM (firmware)

**Negative:**

- Inferred link→part mapping for three listings may need correction when titles are known
- SBOM not yet auto-generated — manual drift risk

**Follow-ups:**

- CycloneDX on release tag; `SECURITY.md`; user fills archived titles for `01ntzGoL`, `0b5MPv1g`, `05YXt20E`
