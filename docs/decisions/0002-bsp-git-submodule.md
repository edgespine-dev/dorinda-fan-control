# ADR-0002: Consume BSP via git submodule

**Status:** Accepted  
**Date:** 2026-07-05

## Context

We need WiFi (and optionally logging/GPIO abstractions) without copying Espressif/Arduino boilerplate into every project. bakerbug-firmware already maintains a tested **Board Support Package** for ESP8266.

Alternatives considered:

| Approach | Pros | Cons |
|----------|------|------|
| **Git submodule** | Pin exact commit; no publish step | Submodule UX friction |
| Copy/paste BSP files | Simple clone | Drift, no upstream fixes |
| PlatformIO lib from registry | Clean deps | BSP not published separately today |
| Fork bakerbug-firmware | Full control | Maintenance burden |

## Decision

Vendor `bakerbug-firmware` as a **git submodule** at `vendor/bakerbug-firmware/`. PlatformIO links `bsp/esp8266` from that path. Pin submodule SHA when tagging releases.

## Consequences

**Positive:**

- Single source of truth for WiFi connect, logging, TLS if needed later
- Platform improvements flow in via submodule bump

**Negative:**

- Developers must `git submodule update --init`
- BSP API changes can break this repo — document required BSP version in release notes

**Follow-ups:**

- Run `git submodule add git@github.com:BakerStack/bakerbug-firmware.git vendor/bakerbug-firmware` on first firmware commit
