# Pull Request

## Summary

<!-- One or two sentences describing the change. -->

## Motivation

<!-- Why is this change needed? Link to related issues. -->

## Changes

- [ ] Code changes under `src/` or `lib/`
- [ ] New or updated tests under `test/native/`
- [ ] Documentation updates under `docs/` or top-level `*.md`
- [ ] CI / build configuration changes
- [ ] No functional change (refactor / chore / docs only)

## Pre-merge checklist

- [ ] `pio test -e native` — all native suites pass
- [ ] `pio run -e xiao_esp32s3_sense` — production firmware compiles
- [ ] `pio run -e xiao_esp32s3_camera_web` — camera-web variant compiles
  (only if affected)
- [ ] Coverage gate (≥ 85 % line / ≥ 70 % branch on `wildlife_core/`)
- [ ] `ruff check tools/` clean
- [ ] `clang-format --dry-run -Werror` clean
- [ ] [docs/testing/regression-plan.md](../docs/testing/regression-plan.md)
  items relevant to this change have been executed
- [ ] `CHANGELOG.md` updated under `[Unreleased]`

## Hardware verification (if applicable)

- [ ] XIAO ESP32S3 boots and joins Wi-Fi
- [ ] Grove Vision AI V2 reachable over I²C (`AT+ID?` returns non-zero ID)
- [ ] `AT+MODELS?` reports `size > 0`
- [ ] `:8080/stream/frame` returns a JPEG (SOI = `FF D8`)
- [ ] MQTT broker receives at least one event

## Risk / rollback

<!-- What is the blast radius? How do you roll back? -->
