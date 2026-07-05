# ADR-0003: No MQTT — WiFi and local web server only

**Status:** Accepted  
**Date:** 2026-07-05

## Context

bakerbug-firmware includes an MQTT stack for fleet/cloud connectivity. The Dorinda fan controller is operated **locally on the boat WiFi**: phone browser → ESP web server. No cloud broker, no telemetry pipeline.

## Decision

Do **not** use MQTT or bakerbug MQTT libraries in this project. Use BSP **WiFi only** plus an on-device HTTP server (ESPAsyncWebServer or equivalent).

## Consequences

**Positive:**

- Smaller firmware, fewer moving parts
- Works on isolated boat WiFi without internet
- Simpler security story (no broker credentials)

**Negative:**

- No remote monitoring when away from the boat unless we add something later (out of scope V1)

**Follow-ups:**

- None unless product requirements change
