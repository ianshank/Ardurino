$ip = "192.168.4.22"
Write-Host "[1] Web UI..."
try {
  $r = Invoke-WebRequest -Uri "http://$ip/" -TimeoutSec 5 -UseBasicParsing
  Write-Host "    OK $($r.StatusCode) $($r.RawContentLength) bytes"
} catch { Write-Host "    ERR $($_.Exception.Message)" }

Write-Host "[2] Issuing INVOKE command..."
try {
  $cmd = "INVOKE=-1,0,0"
  $b = [Convert]::ToBase64String([Text.Encoding]::ASCII.GetBytes($cmd))
  $r = Invoke-WebRequest -Uri "http://$ip/command?base64=$b" -TimeoutSec 8 -UseBasicParsing
  Write-Host "    OK $($r.StatusCode)"
  Write-Host "    body=$($r.Content.Substring(0,[Math]::Min(400,$r.Content.Length)))"
} catch { Write-Host "    ERR $($_.Exception.Message)" }

Write-Host "[3] /stream/frame for 12s..."
try {
  $client = New-Object System.Net.Http.HttpClient
  $client.Timeout = [TimeSpan]::FromSeconds(20)
  $task = $client.GetStreamAsync("http://${ip}:8080/stream/frame")
  if (-not $task.Wait(8000)) {
    Write-Host "    open timeout"
  } else {
    $stream = $task.Result
    $buf = New-Object byte[] 4096
    $total = 0
    $first = $null
    $deadline = (Get-Date).AddSeconds(12)
    while ((Get-Date) -lt $deadline) {
      try { $n = $stream.Read($buf, 0, $buf.Length) } catch { break }
      if ($n -le 0) { break }
      if ($total -eq 0) { $first = [Text.Encoding]::ASCII.GetString($buf, 0, [Math]::Min(200,$n)) }
      $total += $n
    }
    Write-Host "    received $total bytes"
    if ($first) { Write-Host "    head=$first" }
    $stream.Close()
  }
  $client.Dispose()
} catch { Write-Host "    ERR $($_.Exception.Message)" }
