# ADR-0010: Documentation and ADR log before firmware implementation

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Project may sit idle for weeks or months (boat season, other work). Returning without context wastes time re-deriving wiring, test order, and repo boundaries. README alone is not enough — we need **traceable rationale** for non-obvious choices.

## Decision

Before writing TEST 1 firmware:

1. Complete **README**, **hardware**, **roadmap**, **architecture** docs
2. Maintain this **ADR log** (`docs/decisions/`) for significant choices
3. Add new ADR when changing direction (supersede old entries, don't silent-edit)

## Consequences

**Positive:**

- "Why separate repo?" / "Why no MQTT?" answered in one place
- Strong GitHub signal for senior embedded / systems hiring
- Future-you and collaborators onboard faster

**Negative:**

- Small upfront doc cost (already paid)

**Follow-ups:**

- Link ADR index from README
- Add ADR when picking web server library, NVS format, etc.
