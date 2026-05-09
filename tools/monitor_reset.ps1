#requires -Version 5
<#
.SYNOPSIS
    Resets the connected ESP32 board over the serial port and captures the
    boot output to a temp file.
.PARAMETER Port
    Serial port (default: $env:WSPOT_PORT or COM12).
.PARAMETER Seconds
    Seconds to capture (default: 30).
.PARAMETER PioRoot
    PlatformIO root (default: $env:PLATFORMIO_HOME or ~/.platformio).
.PARAMETER OutFile
    Output file (default: <repo-root>/tmp_serial.txt).
#>
param(
    [string]$Port    = $(if ($env:WSPOT_PORT)        { $env:WSPOT_PORT }        else { "COM12" }),
    [int]   $Seconds = 30,
    [string]$PioRoot = $(if ($env:PLATFORMIO_HOME)   { $env:PLATFORMIO_HOME }   else { Join-Path $HOME ".platformio" }),
    [string]$OutFile = $(Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")) "tmp_serial.txt")
)

$ErrorActionPreference = "Stop"
$pio     = Join-Path $PioRoot "penv/Scripts/pio.exe"
$python  = Join-Path $PioRoot "penv/Scripts/python.exe"
$esptool = Join-Path $PioRoot "packages/tool-esptoolpy/esptool.py"

foreach ($exe in @($pio, $python, $esptool)) {
    if (-not (Test-Path $exe)) {
        Write-Error "Missing required executable: $exe (set PLATFORMIO_HOME?)"
        exit 2
    }
}

$job = Start-Job -ArgumentList $pio, $Port -ScriptBlock {
    param($p, $port)
    & $p device monitor -p $port -b 115200 --filter direct 2>&1
}
Start-Sleep -Seconds 3
& $python $esptool --chip esp32s3 --port $Port --no-stub --after hard_reset run 2>&1 |
    Select-Object -Last 2
Start-Sleep -Seconds $Seconds
Stop-Job $job
$out = Receive-Job $job
$out | Out-File $OutFile -Encoding UTF8
Remove-Job $job
Write-Host "---OUTPUT (last 100 lines) — full log: $OutFile ---"
$out | Select-Object -Last 100
