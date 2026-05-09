# End-to-end smoke test

Manual procedure for verifying a fully assembled node (XIAO ESP32S3 Sense
+ Grove Vision AI V2 connected by I²C, both powered, Wi-Fi credentials
provisioned).

## 1. Discover the node

```powershell
# If you know the MAC, sweep the LAN and look it up in ARP.
$mac = "e0-72-a1-f8-77-9c"
1..254 | ForEach-Object {
    (New-Object System.Net.NetworkInformation.Ping).SendPingAsync("192.168.4.$_", 200) | Out-Null
}
Get-NetNeighbor -AddressFamily IPv4 | Where-Object { $_.LinkLayerAddress -eq $mac }
```

## 2. Web UI

```powershell
$ip = "192.168.4.27"
Invoke-WebRequest "http://$ip/" -UseBasicParsing | Select-Object StatusCode, RawContentLength
# Expect: 200, length > 0
```

## 3. AT proxy probes (Grove via XIAO)

```powershell
function ATProxy([string]$ip, [string]$cmd) {
    $b64 = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($cmd))
    Invoke-WebRequest "http://$ip/command?base64=$b64" -UseBasicParsing |
        Select-Object -ExpandProperty Content
}
ATProxy $ip 'AT+ID?'        # non-zero hex ID
ATProxy $ip 'AT+STAT?'      # boot_count, is_ready: 1
ATProxy $ip 'AT+MODELS?'    # MUST report size > 0
ATProxy $ip 'AT+SENSOR?'    # state: 1 once a model is loaded
```

If `MODELS?` shows `size: 0`, follow
[../runbooks/grove-model-flash-pc.md](../runbooks/grove-model-flash-pc.md).

## 4. Stream endpoints (camera-web env only)

```powershell
$r = Invoke-WebRequest "http://${ip}:8080/stream/frame" -UseBasicParsing -TimeoutSec 10
('{0:X2} {1:X2}' -f $r.Content[0], $r.Content[1])  # expect "FF D8"
Invoke-WebRequest "http://${ip}:8080/stream/result" -UseBasicParsing -TimeoutSec 10
```

## 5. MQTT tap

```bash
mosquitto_sub -h <broker> -t 'wildlife/+/event' -v
```

Expect at least one detection event matching
[`data/event.v1.schema.json`](../../data/event.v1.schema.json).

## 6. Post-flash USB verifier (Grove only, USB connected)

```powershell
python tools/grove_flash_check.py COM11
echo "Exit: $LASTEXITCODE"
```

Exit 0 = ready, 2 = descriptor-only flash, 3 = port locked.
