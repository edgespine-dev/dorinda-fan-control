# Firmware

PlatformIO project for Dorinda fan controller.

## Layout

```
firmware/
├── platformio.ini
├── lib/hw/                    # Board-local wrappers (pot, temp, pins)
├── tests/
│   └── test1_sensors/         # TEST 1 — serial only
└── apps/main/                 # (planned) production app
```

**BSP:** `vendor/bakerbug-firmware` (git submodule). Used from TEST 2 onward for WiFi — not linked in TEST 1.

## WiFi credentials (TEST 2+)

```bash
cp secrets.env.example secrets.env   # edit SSID/password
make test2-upload
```

## TEST 1 — build & flash

```bash
make test1              # compile
make test1-upload       # flash D1 Mini
make test1-monitor      # serial @ 115200
# or
make test1-dev          # upload + monitor
```

Expected serial output (once per second):

```
Pot=512 Temp=32.4C
```

No fan PWM in TEST 1 — safe to bench without 12 V connected.

## TEST 2 — WiFi + pot web page

**Boat default:** no shore WiFi — ESP creates its own AP. Phone joins that network.

Credentials: repo-root `secrets.env` (gitignored). `make test2` generates `firmware/include/dfc_secrets.h` from it before compile.

```bash
cp secrets.env.example secrets.env   # WIFI_MODE=AP, SSID, password (min 8 chars)
make test2-upload
make test2-monitor
```

1. Serial shows `AP OK ssid=… ip=192.168.4.1`
2. On phone: join WiFi **`<WIFI_SSID>`** from `secrets.env`
3. Open **`http://192.168.4.1/`** — pot, ADC, temp; auto-refresh 1 s

iOS may show **"Weak Security"** on the AP — normal for ESP8266 (WPA2 only, no WPA3).

JSON API: `/api/status`

## TEST 3 — PWM + current calibration matrix

**Requires 12 V + fan connected.** Pot drives PWM @ 20 kHz (0–100 %).

```bash
make test3-upload
make test3-monitor
```

1. Dashboard: pot → PWM, temp, estimated mA (after cal)
2. **`/cal`** — set PWM knot, enter measured mA from clamp meter, save (persists in LittleFS)
3. Serial dumps full matrix on each save

Suggested knots: 0, 64, 128, 192, 256, 384, 512, 768, 1023.

## Submodule

```bash
git submodule update --init --recursive
```
