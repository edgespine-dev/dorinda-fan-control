# IoT firmware release policy (pragmatic)

Reusable baseline for **Dorinda** and similar BakerStack ESP8266 products until a shared
`edgespine-iot-support` (or vendor signing) layer exists.

**Threat model today:** device runs an **open WiFi AP** on the boat LAN segment. Integrity
and authenticity of the flashed image are **not** cryptographically enforced. Goal is
**traceability** (know exactly what is running), not secure boot.

We document this **on purpose** — not as an oversight. See [Conscious security posture (V1)](#conscious-security-posture-v1) and [DISCLAIMER.md](DISCLAIMER.md).

---

## Conscious security posture (V1)

### What we are aware of

| Risk | V1 reality |
|------|------------|
| Open WiFi AP (`dfan`) | No WPA enterprise, no TLS on HTTP — anyone in radio range can read/control |
| No firmware signing | Flashed image is not cryptographically attested |
| ESP8266 limits | Little headroom for TLS + heavy crypto on every request |
| Supply chain | Submodule + PlatformIO deps traced in [sbom.md](sbom.md); signing deferred |

This is a **deliberate PoC / showcase** release, not an claim of production-grade security.

### Why reduced assurance is accepted for V1

1. **Non-safety-critical function** — enclosure ventilation for an electronics bay. Failure modes are overheating risk to gear or nuisance fan behaviour, not loss of vessel control, navigation, propulsion, or life safety. Worst credible misuse is toggling fan speed or jamming WiFi to the controller.
2. **Maritime context, limited adversary exposure** — the device is installed inside the boat’s electronics space. Practical attackers need physical proximity (marina, anchorage) or already-compromised boat LAN access. Remote internet drive-by attacks against this AP are not the primary threat.
3. **Operator model** — V1 is configured and flashed by the owner; there is no fleet OTA or third-party installer yet. Traceability (git SHA on device + release tags) matters more than anti-tamper for this phase.
4. **Platform economics** — Wemos D1 Mini is chosen for cost and familiarity. Adding secure boot, signed OTA, and always-on TLS would dominate flash/RAM and engineering time without matching the actual risk to people or the vessel in V1.

### What we still do in V1

- **Boot safety** — fan forced OFF until pot or remote arms ([ADR-0015](decisions/0015-boot-safe-fan-off.md))
- **Build identity** — dorinda + BSP git SHA on serial, web footer, `/api/version`
- **HBOM / SBOM** — physical and software traceability for audit conversations ([ADR-0019](decisions/0019-hbom-sbom-showcase.md))
- **Incremental bench sign-off** — [test-log.md](test-log.md)

### When this posture must be revisited

Escalate security work (signing, mTLS, closed provisioning, OTA policy) if any of:

- Device joins a **managed fleet** or cloud control plane (e.g. BakerPilot `firmware_tracks`)
- Function becomes **safety- or security-critical** (bilge, engine, steering, alarms)
- Exposure moves **shore-side** or internet-facing without physical access barrier
- Regulatory or insurer requirements exceed PoC scope

Until then, V1 optimises for **demonstrable awareness + traceability**, not cryptographic hardening.

---

## Version identity

| Layer | Identifier | Where recorded |
|-------|------------|----------------|
| Human release | Git tag `vMAJOR.MINOR[.PATCH]` | GitHub tag, `docs/sbom.md`, release notes |
| App repo | Short git SHA (12 hex) | Build header, `/api/version`, serial boot log |
| BSP submodule | `vendor/bakerbug-firmware` SHA | Same |
| Build moment | UTC timestamp | Build header, `/api/version` |

On-device fields are generated at compile time into `firmware/include/dfc_build_info.h`
(see `Makefile`). After flash, confirm via serial boot line or `GET /api/version`.

---

## When to cut a release

1. **Main app** (`[env:main]`) builds clean: `make main`
2. Bench or boat **sign-off** recorded in [test-log.md](test-log.md) for behaviour touched
3. Submodule pointer is intentional (not accidental `main` tip)
4. [sbom.md](sbom.md) updated if dependencies or submodule pin changed
5. Tag created on **dorinda** repo; submodule SHA noted in tag message / sbom

Patch release (`v1.0.1`): bugfix only, same cal matrix / hardware assumptions.  
Minor release (`v1.1`): new features, API additions (backward compatible).  
Major (`v2.0`): breaking API, pin map, or safety model change.

---

## Release checklist

```text
[ ] git status clean (or only intentional changes)
[ ] vendor/bakerbug-firmware at chosen commit — git submodule status
[ ] make main && make main-upload (or CI equivalent)
[ ] Serial shows: build <tag> dorinda@<sha> bsp@<sha>
[ ] Browser footer or GET /api/version matches expected SHAs
[ ] docs/test-log.md sign-off if hardware behaviour changed
[ ] docs/sbom.md — release tag + BSP SHA
[ ] git tag -a vX.Y.Z -m "..." && git push origin vX.Y.Z
[ ] (optional) GitHub Release with .bin + SHA256 checksum
```

### Submodule pin

At tag time, record both SHAs in [sbom.md](sbom.md):

```text
dorinda-fan-control@<12-char-sha>  tag v1.0
bakerbug-firmware@<12-char-sha>   (submodule)
```

Bump submodule only when BSP/drivers change; rebuild and re-verify WiFi + fan paths.

---

## Flash procedure (operator)

From repo root with `secrets.env` configured:

```bash
make main-dev    # build, upload, monitor
```

Record flashed identity in the boat log (photo of `/api/version` or serial line is enough).

**Do not** rely on “I built yesterday” — always check on-device SHAs after power cycle.

---

## What we explicitly defer

| Topic | Status | Notes |
|-------|--------|-------|
| Firmware image signing | **Deferred** | ESP8266 has no secure boot; signing adds toolchain + key custody |
| Vendor / code-signing cert | **Deferred** | PKI in `bakerlabs-certs` is for **mTLS identity**, not `.bin` signing |
| OTA updates | **Deferred** | `bakerpilot` has `firmware_tracks` schema for future use |
| CycloneDX SBOM artifact | **Planned** | Human [sbom.md](sbom.md) today |
| TLS on local AP | **Out of scope** | Open AP by design for V1 |

Revisit when moving BSP to **edgespine-iot-support** or when devices join a managed fleet
with MQTT + mTLS.

---

## Future alignment (non-blocking)

- Central policy repo: `edgespine-iot-support` or `agent-backbone/policies/iot_release.md`
- Signed artifacts + `firmware_tracks` in BakerPilot control plane
- Pin PlatformIO `lib_deps` to exact versions per release for reproducible SBOM

Until then, **this document + embedded build info + git tags** are the source of truth.
