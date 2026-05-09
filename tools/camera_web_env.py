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
"""

import os

Import("env")


_BUILD_DEFINES = (
    "CAMERA_WIFI_SSID",
    "CAMERA_WIFI_PASSWORD",
    "CAMERA_FALLBACK_AP_SSID",
    "CAMERA_FALLBACK_AP_PASS",
)


def _append_define(name: str, value: str) -> None:
    if not value:
        return
    env.Append(CPPDEFINES=[(name, env.StringifyMacro(value))])


for _name in _BUILD_DEFINES:
    _append_define(_name, os.environ.get(_name, ""))
