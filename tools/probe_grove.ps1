$ip = "192.168.4.22"
function Send-AT($body) {
  $b = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($body))
  Write-Host "[XIAO->Grove] AT+$body"
  try {
    $r = Invoke-WebRequest -Uri "http://$ip/command?base64=$b" -TimeoutSec 10 -UseBasicParsing
    Write-Host "  OK $($r.StatusCode)"
    $body = $r.Content
    if ($body.Length -gt 500) { $body = $body.Substring(0,500) + "..." }
    Write-Host "  $body"
  } catch {
    $resp = $_.Exception.Response
    if ($resp) { Write-Host "  HTTP $([int]$resp.StatusCode)" }
    Write-Host "  ERR $($_.Exception.Message)"
  }
}

Send-AT "ID?"
Send-AT "VER?"
Send-AT "MODELS?"
Send-AT "MODEL?"
Send-AT "SENSOR?"
Send-AT "MODEL=1"
Send-AT "SENSOR=1,1"
Send-AT "MODEL?"
Send-AT "SENSOR?"
Send-AT "INVOKE=1,0,0"
