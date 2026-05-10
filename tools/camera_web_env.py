"""PlatformIO pre-build script for the camera_web_server env.

Injects build-time defines for Wi-Fi credentials and (optionally)
fallback Access Point credentials so the firmware never carries
hard-coded values. All defines are read from environment variables
at build time and are absent when the corresponding variable is unset.

Environment variables read:
    CAMERA_WIFI_SSID         -> -DCAMERA_WIFI_SSID="..."
    CAMERA_WIFI_PASSWORD     -> -DCAMERA_WIFI_PASSWORD="..."
    CAMERA_FALLBACK_AP_SSID  -> -DCAMERA_FALLBACK_AP_SSID="..."
    CAMERA_FALLBACK_AP_PASS  -> -DCAMERA_FALLBACK_AP_PASS="..."
    CAMERA_FALLBACK_AP_OPEN  -> -DCAMERA_FALLBACK_AP_OPEN=1 (dev only; open AP)

The firmware refuses to compile unless either CAMERA_FALLBACK_AP_PASS is
provided or CAMERA_FALLBACK_AP_OPEN=1 is set, so the binary can never ship
with a hard-coded fallback password.
"""

import os
import sys
from collections.abc import Callable
from typing import Any, cast

_scons_import = cast(Callable[..., None], globals()["Import"])
_scons_import("env")
env = cast(Any, globals()["env"])


_STRING_DEFINES = (
    "CAMERA_WIFI_SSID",
    "CAMERA_WIFI_PASSWORD",
    "CAMERA_FALLBACK_AP_SSID",
    "CAMERA_FALLBACK_AP_PASS",
)


def _append_string_define(name: str, value: str) -> None:
    if not value:
        return
    env.Append(CPPDEFINES=[(name, env.StringifyMacro(value))])


def _truthy(value: str) -> bool:
    return value.strip().lower() in {"1", "true", "yes", "on"}


for _name in _STRING_DEFINES:
    _append_string_define(_name, os.environ.get(_name, ""))

# Optional opt-in: explicitly run the fallback AP without authentication.
# Required when CAMERA_FALLBACK_AP_PASS is not set; the firmware will refuse
# to compile otherwise (see src/camera_web_server/main.cpp).
if _truthy(os.environ.get("CAMERA_FALLBACK_AP_OPEN", "")):
    env.Append(CPPDEFINES=[("CAMERA_FALLBACK_AP_OPEN", "1")])
    print(
        "[camera_web_env] WARNING: CAMERA_FALLBACK_AP_OPEN=1 — fallback AP will be unauthenticated.",
        file=sys.stderr,
    )
