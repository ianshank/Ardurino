# Contributing

Thanks for your interest in improving Wildlife Spotter. This project follows
hexagonal architecture: pure C++ logic lives in `lib/wildlife_core/` and is
tested host-side with Unity; Arduino/ESP-specific code lives in
`lib/wildlife_adapters/`.

## Development setup

```powershell
# Python venv for tooling and PlatformIO
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install -U platformio gcovr ruff

# Optional: clang-format 18+ for C++ style checks
```

## Branching

- `main` — protected. Always green CI.
- Feature work: `feat/<short-name>`
- Bug fixes: `fix/<short-name>`
- Docs / chore: `docs/<short-name>`, `chore/<short-name>`

## Required pre-PR checks

| Check                   | Command                                              |
|-------------------------|------------------------------------------------------|
| Native unit tests       | `pio test -e native`                                 |
| Coverage gate           | `pwsh tools/coverage.ps1`                            |
| Embedded build (prod)   | `pio run -e xiao_esp32s3_sense`                      |
| Camera-web build        | `pio run -e xiao_esp32s3_camera_web`                 |
| Static analysis         | `pio check -e xiao_esp32s3_sense --severity=medium`  |
| Python lint             | `ruff check tools/`                                  |
| C++ style               | `clang-format --dry-run -Werror lib/**/*.{cpp,h}`    |

All of these run automatically on pull requests via `.github/workflows/ci.yml`.

## Coverage policy

`gcovr` enforces ≥ 84 % line and ≥ 67 % branch coverage on
`lib/wildlife_core/`. The thresholds are sourced from
`COVERAGE_LINE_THRESHOLD` / `COVERAGE_BRANCH_THRESHOLD` in
[`.github/workflows/ci.yml`](.github/workflows/ci.yml) — update both the
workflow and this document together so they cannot drift. Adapters under
`lib/wildlife_adapters/` are validated by the hardware regression
checklist in
[docs/testing/regression-plan.md](docs/testing/regression-plan.md), not by
line metrics.

## Commit style

Conventional Commits (`feat:`, `fix:`, `docs:`, `chore:`, `refactor:`,
`test:`, `ci:`). Scope is optional but encouraged, e.g.
`feat(publisher): add MQTT QoS-1 retry`.

## Pull request checklist

See [.github/PULL_REQUEST_TEMPLATE.md](.github/PULL_REQUEST_TEMPLATE.md).
