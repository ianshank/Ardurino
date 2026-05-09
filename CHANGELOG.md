# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Fixed
- CI `host-tests-and-coverage` failure: ensure `coverage_html/` exists before
  invoking `gcovr --html-details` (was failing with "No such file or directory").
- CI `embedded-build` failure: prod and dev envs now exclude the vendored
  `src/camera_web_server/` tree (which transitively requires `ArduinoEigen`)
  via `build_src_filter`. Camera-web env still compiles its subtree exclusively.
- CI `format` failure: scoped `clang-format-18 --dry-run -Werror` to the code
  this repo owns (`lib/`, `src/main.cpp`, `test/native/`); vendored Seeed
  source is no longer style-gated.
- CI `lint-python` failure: removed unused file-level `# ruff: noqa: F821`
  from `tools/pio_native_pre.py` and `tools/validate_event_schema.py` and
  replaced them with per-line suppressions on the SCons/PlatformIO
  injected names. CI now pins `ruff==0.15.12`.
- `SscmaI2cSource::begin()`: re-init rate-limiter no longer permanently
  blocks retries when no `IClock` is wired up; gate is bypassed when
  `_clock == nullptr` and applies only when a clock is available.
- `LittleFsConfigStore::read()`: replaced byte-by-byte `read()` loop with a
  single bulk `read(buf, size)` for ~100× fewer flash transactions.
- `MqttTransport::publish()`: previously silently dropped the QoS argument.
  Now logs a one-shot `Warn` via the optional `ILogger*` when QoS > 0 is
  requested, documenting the QoS-0 downgrade.
- `app_httpd.cpp` `proxyCallback`: free `copy` when slot allocation fails
  (memory leak on OOM path).
- `app_httpd.cpp` `results_handler`: when no `image` key is present in the
  upstream JSON, populate `rst_buf` from `slot->data` instead of sending
  uninitialised memory.
- `data/event.v1.schema.json`: stripped accidental UTF-8 BOM.
- `tools/validate_event_schema.py`: read schema and input files with
  `utf-8-sig`; explicit `FileNotFoundError`/`OSError` handling.
- `Publisher::publish` parameter renamed `label_resolver_call_result` →
  `resolved_label`; declaration and definition now agree.

### Changed
- `src/camera_web_server/main.cpp`: fallback AP credentials are now
  build-time defines (`CAMERA_FALLBACK_AP_SSID`, `CAMERA_FALLBACK_AP_PASS`)
  injected via `tools/camera_web_env.py` from environment variables. AP
  password is no longer printed to the serial console.
- `tools/monitor_reset.ps1`, `tools/capture_serial.ps1`: removed all
  hard-coded user paths; PlatformIO root, port, output file all parameterised
  via env vars or `param()` defaults.
- `README.md`: PlatformIO version requirement aligned with CI (≥ 6.1.15).
- `docs/testing/regression-plan.md` §6: schema validation example now uses
  `data/event.v1.example.json` (added).

### Removed
- `tools/probe_grove.ps1`, `probe_grove2.ps1`, `probe_stream.ps1`: scratch
  scripts with hard-coded LAN IPs (use `pio device monitor` instead).
- `.vscode/c_cpp_properties.json`, `.vscode/launch.json`: PlatformIO-generated
  per-developer files; now `.gitignore`d.

### Added
- `data/event.v1.example.json` — canonical example payload referenced by the
  regression plan and used as a smoke input for the JSON-Schema validator.
- `docs/architecture/` C4 model: context, container, and component diagrams.
- `docs/runbooks/grove-model-flash-pc.md` — PC-only model flashing playbook
  for the Grove Vision AI V2 (no mobile app required).
- `docs/testing/regression-plan.md` — pre-merge regression checklist covering
  native unit tests, embedded build smoke, and hardware bring-up.
- `docs/operations/smoke-test.md` — end-to-end LAN/I²C/HTTP smoke procedure.
- `tools/grove_flash_check.py` — post-flash verifier that reads `AT+ID?` and
  `AT+MODELS?` over USB serial and exits non-zero when only the model
  descriptor (size = 0) was written.
- `xiao_esp32s3_camera_web` PlatformIO env for the Seeed camera-web-server
  variant, including `tools/camera_web_env.py` for credential injection.
- `CONTRIBUTING.md`, `SECURITY.md`, `LICENSE` (MIT).
- GitHub issue and pull-request templates under `.github/`.

### Changed
- `README.md` rewritten with quick-start, architecture index, troubleshooting,
  and roadmap sections. Removed stale references to internal `memories/`.
- `.gitignore` hardened: ignores Python venvs, build artifacts, ad-hoc
  `tmp_*` / `test_*_out*` scratch logs, and editor caches.
- Repository reorganised into an enterprise layout (`docs/{architecture,
  runbooks, operations, testing}`) without disturbing the firmware sources
  under `src/`, `lib/`, or `test/`.

### Fixed
- Diagnosed and documented the "MODELS? size:0" descriptor-only flash failure
  on Grove Vision AI V2 (browser focus loss, wrong board selection in the
  SenseCraft Web Toolkit dropdown, stale CH343 driver).

### Known Issues
- Grove Vision AI V2 currently reports `MODELS? size:0` after a Web Toolkit
  flash on this bench. Streaming endpoints (`:8080/stream/frame`,
  `:8080/stream/result`) correctly block until the model binary is present.
  See `docs/runbooks/grove-model-flash-pc.md` for the recovery procedure.
