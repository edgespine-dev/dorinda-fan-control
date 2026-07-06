.DEFAULT_GOAL := help

# Prefer: explicit PIO=, then PATH, then PlatformIO home, then project .venv, then bakerbug sibling
ROOT := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
PIO ?= $(shell command -v pio 2>/dev/null)
ifeq ($(PIO),)
  PLATFORMIO_PIO := $(HOME)/.platformio/penv/bin/pio
  ifneq ($(wildcard $(PLATFORMIO_PIO)),)
    PIO := $(PLATFORMIO_PIO)
  else ifneq ($(wildcard $(ROOT)/.venv/bin/pio),)
    PIO := $(ROOT)/.venv/bin/pio
  else ifneq ($(wildcard $(ROOT)/../bakerbug-firmware/.venv/bin/pio),)
    PIO := $(ROOT)/../bakerbug-firmware/.venv/bin/pio
  else
    PIO := pio
  endif
endif
export PLATFORMIO_CORE_DIR ?= $(HOME)/.platformio

FW       := firmware
DFC_SRC  := tests/test1_sensors/src
export DFC_SRC

# WiFi credentials for TEST 2+ (copy secrets.env.example → secrets.env)
SECRETS_ENV := secrets.env
SECRETS_HDR := $(FW)/include/dfc_secrets.h
WIFI_MODE ?= AP
WIFI_SSID ?=
WIFI_PASSWORD ?=

ifneq ($(wildcard $(SECRETS_ENV)),)
include $(SECRETS_ENV)
endif

# Generate C header from secrets.env (reliable — no -D escaping issues)
$(SECRETS_HDR): $(SECRETS_ENV)
	@mkdir -p $(FW)/include
	@printf '%s\n' \
		'/* generated from secrets.env — do not commit */' \
		'#pragma once' \
		'#define DFC_WIFI_MODE "$(WIFI_MODE)"' \
		'#define DFC_WIFI_SSID "$(WIFI_SSID)"' \
		'#define DFC_WIFI_PASSWORD "$(WIFI_PASSWORD)"' \
		> $@

TEST2_RUN = cd $(FW) && DFC_SRC=tests/test2_wifi_pot/src $(PIO) run -e test2
TEST3_RUN = cd $(FW) && DFC_SRC=tests/test3_pwm_matrix/src $(PIO) run -e test3
MAIN_RUN  = cd $(FW) && DFC_SRC=apps/main/src $(PIO) run -e main

.PHONY: help test1 test1-upload test1-monitor test1-dev test2 test2-upload test2-monitor test2-dev test3 test3-upload test3-monitor test3-dev main main-upload main-monitor main-dev main-erase clean init sync-bsp-libdeps

# PlatformIO copies path-based lib_deps into .pio/libdeps/ and can serve stale BSP.
sync-bsp-libdeps:
	@rm -rf $(FW)/.pio/libdeps/*/bsp

# Full chip erase — clears stale RF-cal / PHY-init / SDK param sectors.
# Use when AP reports OK in serial but no device sees the network.
main-erase:
	$(MAIN_RUN) -t erase

help:
	@echo "dorinda-fan-control"
	@echo ""
	@echo "  make init           Create .venv with PlatformIO (optional)"
	@echo "  make test1          Build TEST 1 (pot + temp → serial)"
	@echo "  make test1-upload   Flash TEST 1"
	@echo "  make test1-monitor  Serial monitor (115200)"
	@echo "  make test1-dev      upload + monitor"
	@echo "  make test2          Build TEST 2 (WiFi + pot web page)"
	@echo "  make test2-upload   Flash TEST 2"
	@echo "  make test2-monitor  Serial monitor (115200)"
	@echo "  make test2-dev      upload + monitor"
	@echo "  make test3          Build TEST 3 (PWM + cal matrix)"
	@echo "  make test3-upload   Flash TEST 3"
	@echo "  make test3-monitor  Serial monitor (115200)"
	@echo "  make test3-dev      upload + monitor"
	@echo "  make main           Build main app (REST + pot)"
	@echo "  make main-upload    Flash main app"
	@echo "  make main-erase     Full chip erase (fix stale RF-cal → dead AP)"
	@echo "  make main-monitor   Serial monitor (115200)"
	@echo "  make main-dev       upload + monitor"
	@echo "  make clean          Remove build artifacts"

init:
	python3 -m venv .venv
	.venv/bin/pip install -q platformio
	$(MAKE) PIO=.venv/bin/pio test1

test1:
	cd $(FW) && $(PIO) run -e test1

test1-upload:
	cd $(FW) && $(PIO) run -e test1 -t upload

test1-monitor:
	cd $(FW) && $(PIO) device monitor -b 115200

test1-dev: test1-upload test1-monitor

test2: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "TEST 2 build — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(TEST2_RUN)

test2-upload: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "TEST 2 upload — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(TEST2_RUN) -t upload

test2-monitor:
	cd $(FW) && $(PIO) device monitor -b 115200

test2-dev: test2-upload test2-monitor

test3: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "TEST 3 build — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(TEST3_RUN)

test3-upload: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "TEST 3 upload — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(TEST3_RUN) -t upload

test3-monitor:
	cd $(FW) && $(PIO) device monitor -b 115200

test3-dev: test3-upload test3-monitor

main: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "Main app build — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(MAIN_RUN)

main-upload: $(SECRETS_HDR) sync-bsp-libdeps
	@test -n "$(WIFI_SSID)" || (echo "ERROR: WIFI_SSID missing in secrets.env" && exit 1)
	@test -n "$(WIFI_PASSWORD)" || (echo "ERROR: WIFI_PASSWORD missing in secrets.env" && exit 1)
	@echo "Main app upload — MODE=$(WIFI_MODE) SSID=$(WIFI_SSID)"
	$(MAIN_RUN) -t upload

main-monitor:
	cd $(FW) && $(PIO) device monitor -b 115200

main-dev: main-upload main-monitor

clean: sync-bsp-libdeps
	cd $(FW) && $(PIO) run -e test1 -t clean
	cd $(FW) && DFC_SRC=tests/test2_wifi_pot/src $(PIO) run -e test2 -t clean
	cd $(FW) && DFC_SRC=tests/test3_pwm_matrix/src $(PIO) run -e test3 -t clean
	cd $(FW) && DFC_SRC=apps/main/src $(PIO) run -e main -t clean
