import os

Import("env")


def _append_define(name: str, value: str) -> None:
    if value:
        env.Append(CPPDEFINES=[(name, env.StringifyMacro(value))])


_append_define("CAMERA_WIFI_SSID", os.environ.get("CAMERA_WIFI_SSID", ""))
_append_define("CAMERA_WIFI_PASSWORD", os.environ.get("CAMERA_WIFI_PASSWORD", ""))
