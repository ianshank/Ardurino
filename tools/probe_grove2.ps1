$ip = "192.168.4.22"
function Send-AT($body) {
  $b = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($body))
  Write-Host "[XIAO->Grove] AT+$body"
  try {
    $r = Invoke-WebRequest -Uri "http://$ip/command?base64=$b" -TimeoutSec 12 -UseBasicParsing
    $body = $r.Content
    if ($body.Length -gt 700) { $body = $body.Substring(0,700) + "..." }
    Write-Host "  $body"
  } catch {
    Write-Host "  ERR $($_.Exception.Message)"
  }
}

Send-AT "SENSORS?"
Send-AT "INFO?"
Send-AT "ALGOS?"
Send-AT "ALGO?"
Send-AT "STAT?"
# Try various SENSOR forms
Send-AT "SENSOR=1"
Send-AT "SENSOR=1,1,0"
Send-AT "SENSOR?"
# Try INVOKE after sensor is enabled
Send-AT "INVOKE=1,0,0"
