# Regression Test Plan

This plan is the gate that **every** pull request must pass before merging
to `main`. CI executes the host-side checks automatically; the
hardware-in-the-loop section is run on a bench by the reviewer or the
submitter and attached to the PR.

## 1. Static checks (CI)

| ID    | Check                                | Command                                                  |
|-------|--------------------------------------|----------------------------------------------------------|
| S-01  | C++ formatting                       | `clang-format --dry-run -Werror lib/**/*.{cpp,h}`        |
| S-02  | Static analysis (medium+)            | `pio check -e xiao_esp32s3_sense --severity=medium`      |
| S-03  | Python lint                          | `ruff check tools/`                                      |

## 2. Host unit tests (CI)

Run via `pio test -e native`. Current baseline: **172 / 172** passing
across 15 Unity suites.

| ID    | Suite                       | Asserts                                            |
|-------|-----------------------------|----------------------------------------------------|
| U-01  | `test_state_machine`        | Boot → detect → sleep transitions                  |
| U-02  | `test_state_machine_ota`    | OTA happy path + rollback                          |
| U-03  | `test_debouncer`            | Flap suppression window                            |
| U-04  | `test_serializer`           | `event.v1.schema.json` conformance                 |
| U-05  | `test_publisher`            | Batched publish + transport-failure handling       |
| U-06  | `test_retry_policy`         | Exp backoff bounds + jitter                        |
| U-07  | `test_labels`               | `labels.json` resolver                             |
| U-08  | `test_config`               | LittleFS config load + defaults fallback           |
| U-09  | `test_log_helpers`          | Level filtering + structured output                |
| U-10  | `test_json` (mini_json)     | Zero-alloc parser edge cases                       |
| U-11  | `test_chunker`              | JPEG → MQTT chunks, ordering                       |
| U-12  | `test_runtime_enums`        | Enum stringification round-trip                    |
| U-13  | `test_semver`               | Comparator for OTA decisions                       |
| U-14  | `test_inference_filter`     | Confidence threshold + class allow/deny            |
| U-15  | `test_adapters_host`        | Adapter contracts compile + smoke under host gcc   |

## 3. Coverage gate (CI)

```powershell
pwsh tools/coverage.ps1
```

- Filter: `lib/wildlife_core/`
- Thresholds: ≥ 84 % line, ≥ 67 % branch (enforced by `gcovr --fail-under-*`)
- Report: `coverage.xml` + `coverage_html/` uploaded as CI artifact.

## 4. Embedded build smoke (CI)

| ID   | Env                              | Command                                      |
|------|----------------------------------|----------------------------------------------|
| B-01 | `xiao_esp32s3_sense`             | `pio run -e xiao_esp32s3_sense`              |
| B-02 | `xiao_esp32s3_sense_dev`         | `pio run -e xiao_esp32s3_sense_dev`          |
| B-03 | `xiao_esp32s3_camera_web`        | `pio run -e xiao_esp32s3_camera_web`         |
| B-04 | `xiao_esp32s3_camera_web_fake`   | `pio run -e xiao_esp32s3_camera_web_fake`    |

Failure modes to watch for:

- LittleFS image overflow (`partitions/dual_ota_littlefs.csv` is tight).
- ArduinoEigen pulled in for non-camera-web envs (regression in `lib_deps`).

## 5. Hardware-in-the-loop bring-up (bench)

Run end-to-end after touching anything in `wildlife_adapters/`, `src/`,
`tools/camera_web_env.py`, or partition tables. Record results in the PR.

| ID   | Step                                                   | Pass criterion                                                                  |
|------|--------------------------------------------------------|---------------------------------------------------------------------------------|
| H-01 | Flash XIAO: `pio run -e xiao_esp32s3_sense -t upload`  | Upload OK, device reboots                                                       |
| H-02 | Wi-Fi join (captive portal or stored creds)            | Device gets DHCP lease, MAC visible on AP                                       |
| H-03 | `GET http://<xiao-ip>/` (web UI)                       | 200 OK, body > 0 bytes                                                          |
| H-04 | AT proxy: `AT+ID?` to Grove via XIAO                   | Returns non-zero hex ID                                                         |
| H-05 | AT proxy: `AT+MODELS?`                                 | `size > 0` for at least one model entry                                         |
| H-06 | Production `GET :8080/stream/frame` after web UI Start | Body starts with `FF D8` (JPEG SOI)                                             |
| H-07 | `GET :8080/stream/result`                              | Valid SSCMA result JSON                                                         |
| H-08 | MQTT broker tap (`mosquitto_sub`)                      | At least one event published end-to-end                                         |
| H-09 | `python tools/grove_flash_check.py COM<n>`             | Exit code 0 (`size > 0`)                                                        |
| H-10 | Fake env direct MJPEG smoke                            | Script sees ≥ 3 frames in 3 s; no Grove.                                      |

## 6. Schema validation

```powershell
python tools/validate_event_schema.py data/event.v1.example.json
```

Should be run whenever `event_serializer.cpp` or the schema changes.

## 7. Known good baseline

CI re-asserts the host-side baseline on every PR; do not encode per-PR
status in this document (it goes stale as soon as another PR lands). The
baseline below describes the green-state shape — see the GitHub Actions
run linked from the PR for the current numbers.

- Static + lint: clean (`format`, `lint-python` jobs green).
- Native tests: full Unity suite passing (`pio test -e native`, see the
  `host-tests-and-coverage` job for the current case count).
- Coverage gate: ≥ 84 % line, ≥ 67 % branch on `lib/wildlife_core/`.
- Embedded builds: all four matrix envs (`xiao_esp32s3_sense`,
  `xiao_esp32s3_sense_dev`, `xiao_esp32s3_camera_web`,
  `xiao_esp32s3_camera_web_fake`) compile + pass `pio check`.
- Hardware: H-01..H-04 and H-06..H-08 verified on bench; **H-05 currently
  fails** (`MODELS? size: 0`) — see
  [../runbooks/grove-model-flash-pc.md](../runbooks/grove-model-flash-pc.md).
