[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [Alias('Host')]
    [string]$DeviceHost,

    [int]$Port = 8080,
    [int]$Seconds = 3,
    [int]$MinFrames = 3
)

$ErrorActionPreference = 'Stop'

if ($DeviceHost -match '^https?://') {
    $baseUri = $DeviceHost.TrimEnd('/')
} else {
    $baseUri = "http://$DeviceHost`:$Port"
}

$streamUri = "$baseUri/stream/frame"
$curl = Get-Command curl.exe -ErrorAction SilentlyContinue
if (-not $curl) {
    throw 'curl.exe is required for bounded MJPEG smoke capture.'
}

$tmp = New-TemporaryFile
try {
    & $curl.Source --silent --show-error --max-time $Seconds --output $tmp.FullName $streamUri
    $curlExit = $LASTEXITCODE
    if ($curlExit -ne 0 -and $curlExit -ne 28) {
        throw "curl.exe failed with exit code $curlExit while reading $streamUri"
    }

    $bytes = [System.IO.File]::ReadAllBytes($tmp.FullName)
    $soi = 0
    $eoi = 0
    for ($i = 0; $i -lt ($bytes.Length - 1); $i++) {
        if ($bytes[$i] -eq 0xFF -and $bytes[$i + 1] -eq 0xD8) {
            $soi++
        }
        if ($bytes[$i] -eq 0xFF -and $bytes[$i + 1] -eq 0xD9) {
            $eoi++
        }
    }

    $frames = [Math]::Min($soi, $eoi)
    if ($frames -lt $MinFrames) {
        throw "Expected at least $MinFrames JPEG frames from $streamUri, got $frames (SOI=$soi, EOI=$eoi, bytes=$($bytes.Length))."
    }

    Write-Host "MJPEG smoke OK: $frames frame(s), $($bytes.Length) bytes from $streamUri"
} finally {
    Remove-Item -Force $tmp.FullName -ErrorAction SilentlyContinue
}
