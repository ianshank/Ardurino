# Security Policy

## Supported versions

This is a hobby/research firmware. Only the `main` branch receives security
fixes.

## Reporting a vulnerability

Please open a private security advisory via GitHub:

1. Go to <https://github.com/ianshank/Ardurino/security/advisories/new>
2. Provide a clear reproduction, affected commit, and impact.

Do **not** open a public issue for credential exposure, RCE, or
authentication-bypass reports. Field-deployed wildlife nodes may be
internet-reachable through their MQTT broker, so credential issues are
treated as high severity.

## Out of scope

- Physical access attacks (the device exposes BOOT/RESET pins by design).
- Vulnerabilities in upstream libraries (`Seeed_Arduino_SSCMA`, `PubSubClient`,
  `WiFiManager`, `ArduinoJson`); please report to those upstreams.
