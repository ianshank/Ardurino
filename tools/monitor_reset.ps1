param(
  [string]$Port = "COM12",
  [int]$Seconds = 30
)
$pio = "C:\Users\iansh\.platformio\penv\Scripts\pio.exe"
$python = "C:\Users\iansh\.platformio\penv\Scripts\python.exe"
$esptool = "C:\Users\iansh\.platformio\packages\tool-esptoolpy\esptool.py"

$job = Start-Job -ArgumentList $pio,$Port -ScriptBlock {
  param($p, $port)
  & $p device monitor -p $port -b 115200 --filter direct 2>&1
}
Start-Sleep -Seconds 3
& $python $esptool --chip esp32s3 --port $Port --no-stub --after hard_reset run 2>&1 | Select-Object -Last 2
Start-Sleep -Seconds $Seconds
Stop-Job $job
$out = Receive-Job $job
$out | Out-File C:\Users\iansh\OneDrive\Documents\Ardurino\tmp_serial.txt -Encoding UTF8
Remove-Job $job
Write-Host "---OUTPUT (last 100 lines)---"
$out | Select-Object -Last 100
