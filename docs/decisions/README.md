# Design Decision Log (ADR)

Architecture Decision Records for **dorinda-fan-control**.

Read this when you wonder *"why did we do it this way?"* — especially after a long break.

## Format

Each ADR follows a lightweight template:

| Section | Purpose |
|---------|---------|
| **Status** | `Accepted` · `Proposed` · `Superseded` · `Deprecated` |
| **Date** | When the decision was made |
| **Context** | Problem or forces at play |
| **Decision** | What we chose |
| **Consequences** | Trade-offs, what becomes easier/harder |

## Index

| ADR | Title | Status |
|-----|-------|--------|
| [0001](0001-separate-repository.md) | Separate repo instead of bakerbug-firmware monorepo | Accepted |
| [0002](0002-bsp-git-submodule.md) | Consume BSP via git submodule | Accepted |
| [0003](0003-no-mqtt.md) | No MQTT — WiFi/web only | Accepted |
| [0004](0004-incremental-test-phases.md) | TEST 1 → 2 → 3 before main app | Accepted |
| [0005](0005-platform-esp8266-d1-mini.md) | ESP8266 Wemos D1 Mini as sole MCU (V1) | Accepted |
| [0006](0006-pwm-current-calibration-matrix.md) | PWM→current via measured calibration matrix | Accepted |
| [0007](0007-override-drop-on-pot-move.md) | Web override clears when pot moves | Accepted |
| [0008](0008-safety-thresholds-v2.md) | Thermal safety (70 °C / 85 °C) in main app V2 | Accepted |
| [0009](0009-esp-now-future-climate-node.md) | Future ESP-NOW remote sensor — separate node/repo | Accepted |
| [0010](0010-docs-before-code.md) | Documentation and ADR log before firmware implementation | Accepted |
| [0011](0011-local-hw-layer-not-bsp-yet.md) | Local `hw/` layer; DS18B20/MOSFET BSP deferred | Accepted |
| [0012](0012-hw-component-api.md) | Driver API — init, set, get, loop (`drivers::`) | Accepted |
| [0013](0013-promote-hw-drivers-to-bakerbug.md) | Pot, DS18B20, fan_control in bakerbug `drivers/`; Dorinda = pins + init | Accepted |
| [0014](0014-cooperative-loop-timing.md) | 1 s control / 5 s temp — cooperative loop, WiFi-first boot | Accepted |
| [0015](0015-boot-safe-fan-off.md) | Fan PWM off until pot >4 % or remote arms | Accepted |
| [0016](0016-embedded-cal-matrix-main.md) | Main app: embedded bench matrix, no LittleFS | Accepted |
| [0017](0017-dfc-wifi-shared-boot.md) | Shared `dfc_wifi` AP/STA boot across tests and main | Accepted |
| [0018](0018-fan-quick-presets.md) | OFF / Silent (~1 V) / FULL quick presets on web UI | Accepted |
| [0019](0019-hbom-sbom-showcase.md) | HBOM + SBOM docs for showcase traceability | Accepted |

## Adding a new decision

1. Copy the template below into `docs/decisions/NNNN-short-title.md`
2. Add a row to the index table above
3. Link from README if the decision is user-visible

### Template

```markdown
# ADR-NNNN: Title

**Status:** Proposed
**Date:** YYYY-MM-DD

## Context

…

## Decision

…

## Consequences

**Positive:** …
**Negative:** …
**Follow-ups:** …
```
