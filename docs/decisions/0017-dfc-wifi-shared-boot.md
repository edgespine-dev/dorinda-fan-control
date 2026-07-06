# ADR-0017: Shared `dfc_wifi` library for AP/STA boot

**Status:** Accepted  
**Date:** 2026-07-06

## Context

TEST 2, TEST 3, and main app all need the same WiFi behaviour: `WiFi.persistent(false)`, no auto-reconnect, `WIFI_NONE_SLEEP`, and consistent softAP parameters (channel 1, visible SSID, max 4 stations). Duplicated setup code diverged and made AP regressions hard to bisect.

BSP WiFi was extended with `bsp_wifi_softap_begin()`, station count, IP, and `bsp_wifi_print_diag()`.

## Decision

Add **`firmware/lib/dfc_wifi/`** — thin Dorinda wrapper:

- `dfc_wifi_prepare()` — global WiFi flags
- `dfc_wifi_boot()` — AP or STA from `secrets.env` (`DFC_WIFI_*`)
- `dfc_wifi_ready()`, `dfc_wifi_ip()`, `dfc_wifi_client_count()`

TEST 2, TEST 3, and main call the same entry points. **Do not** tear down and restart the AP from `loop()` — if AP is lost, power-cycle or fix root cause; automatic AP restart caused flapping during bring-up.

## Consequences

**Positive:**

- One place to match working TEST 2 AP settings
- Serial diagnostics via BSP `log_printf` + `WiFi.printDiag()`
- Binary-search debugging: same env, swap source tree

**Negative:**

- Dorinda-specific lib (not in bakerbug) — acceptable; secrets and SSID policy are product-local

**Follow-ups:**

- Promote generic softAP helpers further into bakerbug BSP if other BakerStack products need identical AP mode
