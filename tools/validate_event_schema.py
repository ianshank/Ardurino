#!/usr/bin/env python3
"""validate_event_schema.py — validate a detection event JSON string/file against event.v1.schema.json.

Usage:
    python tools/validate_event_schema.py '{"schema":"detection.v1",...}'
    python tools/validate_event_schema.py path/to/event.json
    python tools/validate_event_schema.py --stdin  (reads from stdin)

Exit codes:
    0  — valid
    1  — validation error
    2  — usage / file-not-found error
    3  — jsonschema not installed (soft skip in CI)
"""

import json
import pathlib
import sys

SCHEMA_PATH = pathlib.Path(__file__).parent.parent / "data" / "event.v1.schema.json"


def load_schema() -> dict:
    # utf-8-sig tolerates an accidental BOM in the schema file without
    # failing the whole pipeline.
    try:
        with SCHEMA_PATH.open(encoding="utf-8-sig") as fh:
            return json.load(fh)
    except FileNotFoundError:
        print(f"Schema file not found: {SCHEMA_PATH}", file=sys.stderr)
        sys.exit(2)


def parse_input(args: list[str]) -> str:
    if not args:
        print("Usage: validate_event_schema.py <json-string | file.json | --stdin>", file=sys.stderr)
        sys.exit(2)

    if args[0] == "--stdin":
        return sys.stdin.read()

    candidate = pathlib.Path(args[0])
    if candidate.is_file():
        try:
            return candidate.read_text(encoding="utf-8-sig")
        except OSError as exc:
            print(f"Could not read file '{candidate}': {exc}", file=sys.stderr)
            sys.exit(2)

    # Treat as a raw JSON string
    return args[0]


def main() -> None:
    try:
        import jsonschema  # type: ignore[import-untyped]
    except ImportError:
        print(
            "WARNING: jsonschema not installed — skipping event schema validation.",
            file=sys.stderr,
        )
        sys.exit(3)

    raw = parse_input(sys.argv[1:])

    try:
        instance = json.loads(raw)
    except json.JSONDecodeError as exc:
        print(f"JSON parse error: {exc}", file=sys.stderr)
        sys.exit(1)

    schema = load_schema()

    validator_cls = jsonschema.Draft202012Validator
    validator = validator_cls(schema)
    errors = sorted(validator.iter_errors(instance), key=lambda e: str(e.path))

    if errors:
        print(f"Validation FAILED ({len(errors)} error(s)):", file=sys.stderr)
        for err in errors:
            path = " > ".join(str(p) for p in err.absolute_path) or "(root)"
            print(f"  [{path}] {err.message}", file=sys.stderr)
        sys.exit(1)

    print("Validation OK")


if __name__ == "__main__":
    main()
