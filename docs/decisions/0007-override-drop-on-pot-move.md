# ADR-0007: Web override clears when physical pot moves

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Main app allows **web slider override** of fan speed for convenience at the helm. The **physical pot remains installed** as local authority and fallback. We need a rule for when override ends.

Options:

| Rule | Behaviour |
|------|-----------|
| Timeout only | Override expires after N minutes |
| Explicit "release" button | User must tap to return to pot |
| **Pot movement** | Any intentional pot change cancels override |

## Decision

**Pot movement cancels override.** Compare current ADC to value captured when override was engaged; if delta exceeds **DEADBAND**, clear override and return to pot mapping.

DEADBAND is tuned from TEST 1 logs (pot noise at fixed position). Start with ~3–5 % of ADC range.

## Consequences

**Positive:**

- Matches user expectation: "I turned the knob, it listens to me"
- No stale override when someone adjusts hardware

**Negative:**

- Vibrations/noise near deadband could flicker override — TEST 1 quantifies this

**Follow-ups:**

- Optional software filtering on pot ADC (EMA) if noise is high
