# ADR-0001: Separate repository instead of bakerbug-firmware monorepo

**Status:** Accepted  
**Date:** 2026-07-05

## Context

Fan controller firmware is a **Dorinda-specific product** (12 V enclosure, pot + DS18B20 + MOSFET). The bakerbug-firmware repo is a shared IoT platform (BSP, MQTT, provisioning roadmap). Mixing a boat fan controller into that monorepo would:

- Blur product boundaries
- Pull Dorinda wiring/HW docs into a generic firmware repo
- Make the GitHub story harder to tell ("embedded product" vs "platform")

An early prototype (`fan_control` under `bakerbug-firmware/apps/`) was useful for spike work but is not the long-term home.

## Decision

Create **`dorinda-fan-control`** as a standalone repository sibling to `bakerbug-firmware`. Product logic, HW docs, CAD, web UI, and calibration data live here.

## Consequences

**Positive:**

- Clean GitHub showcase: one repo = one product
- README and roadmap speak to recruiters/collaborators without BakerBug context
- Release cadence independent of platform changes

**Negative:**

- Must manage BSP version (submodule pin) explicitly
- Two repos to clone for full dev setup

**Follow-ups:**

- Remove or archive `bakerbug-firmware/apps/fan_control` spike when TEST 1 lands here
