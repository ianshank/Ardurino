#requires -Version 5
<#
.SYNOPSIS
    Resets the connected ESP32 board and captures serial output for $Seconds.
.PARAMETER Port
    Serial port (default: $env:WSPOT_PORT or COM12).
.PARAMETER Seconds
    Seconds to capture (default: 20).
.PARAMETER PioRoot
    PlatformIO root (default: $env:PLATFORMIO_HOME or ~/.platformio).
.PARAMETER OutFile
    Output file (default: <repo-root>/tmp_serial.txt).
#>
param(
    [string]$Port    = $(if ($env:WSPOT_PORT)      { $env:WSPOT_PORT }      else { "COM12" }),
    [int]   $Seconds = 20,
    [string]$PioRoot = $(if ($env:PLATFORMIO_HOME) { $env:PLATFORMIO_HOME } else { Join-Path $HOME ".platformio" }),
    [string]$OutFile = $(Join-Path (Resolve-Path (Join-Path $PSScriptRoot "..")) "tmp_serial.txt")
)

$ErrorActionPreference = "Stop"
$python  = Join-Path $PioRoot "penv/Scripts/python.exe"
$esptool = Join-Path $PioRoot "packages/tool-esptoolpy/esptool.py"

foreach ($exe in @($python, $esptool)) {
    if (-not (Test-Path $exe)) {
        Write-Error "Missing required executable: $exe (set PLATFORMIO_HOME?)"
        exit 2
    }
}

# Hard reset (no-stub mode avoids stub upload failure on this board).
& $python $esptool --chip esp32s3 --port $Port --no-stub --after hard_reset run 2>&1 |
    Select-Object -Last 3
Start-Sleep -Milliseconds 800

try {
    $sp              = New-Object System.IO.Ports.SerialPort $Port, 115200, None, 8, One
    $sp.ReadTimeout  = 500
    $sp.Open()
    $sp.DtrEnable    = $true
    Write-Host "[host] port opened, listening..."
    $sb       = New-Object System.Text.StringBuilder
    $deadline = (Get-Date).AddSeconds($Seconds)
    while ((Get-Date) -lt $deadline) {
        try {
            $line = $sp.ReadLine()
            $null = $sb.AppendLine($line)
            Write-Host $line
        } catch [System.TimeoutException] { }
    }
    $sp.Close()
    $sb.ToString() | Out-File $OutFile -Encoding UTF8
    Write-Host "---Captured $($sb.Length) chars to $OutFile---"
} catch {
    Write-Host "ERR: $($_.Exception.Message)"
    exit 1
}
