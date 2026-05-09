# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
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
