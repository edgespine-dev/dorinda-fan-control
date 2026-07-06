# ADR-0020: V1 security posture — conscious risk acceptance

**Status:** Accepted  
**Date:** 2026-07-06

## Context

Dorinda fan control V1 is a **public showcase / PoC**: open WiFi AP, plain HTTP, no firmware
signing, ESP8266 with limited crypto headroom. Reviewers (CRA-adjacent traceability, insurers,
collaborators) may ask whether security was ignored.

It was not. The product controls **non-safety-critical** enclosure ventilation on a private
sailboat installation with **low remote attacker exposure**.

## Decision

1. **Document** known gaps and rationale in [RELEASE.md](../RELEASE.md#conscious-security-posture-v1).
2. **Ship V1** with traceability (git SHA on device, tags, HBOM/SBOM) and boot-safe fan policy — not with secure boot or signed OTA.
3. **Revisit** when scope becomes fleet-managed, safety-critical, or internet-exposed.

## Rationale (summary)

| Factor | V1 choice |
|--------|-----------|
| Harm potential | Fan speed / bay cooling — not nav, engine, bilge, or life safety |
| Threat exposure | Physical or LAN proximity on board; not a cloud-facing production fleet |
| Platform | D1 Mini — pragmatic; heavy security stack deferred |
| Showcase goal | Prove architecture, cal workflow, and **documented** supply-chain awareness |

## Consequences

**Positive:**

- Honest posture for auditors: risks named, mitigations and deferrals explicit
- Engineering effort stays on bring-up, calibration, and operator UX

**Negative:**

- Open AP remains abusable by anyone in WiFi range at the marina
- No cryptographic proof of firmware integrity on flash

**Follow-ups:**

- Link to central IoT policy when `edgespine-iot-support` exists
- Signing / OTA when BakerPilot fleet model applies
