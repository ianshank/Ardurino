# Wildlife Spotter — local coverage runner.
# Mirrors what CI runs so contributors can iterate without waiting on a PR.

[CmdletBinding()]
param(
    [int]$FailUnderLine   = 85,
    [int]$FailUnderBranch = 70
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot

Push-Location $root
try {
    Write-Host "==> pio test -e native" -ForegroundColor Cyan
    pio test -e native --verbose
    if ($LASTEXITCODE -ne 0) { throw "Native tests failed." }

    Write-Host "==> gcovr coverage gate" -ForegroundColor Cyan
    $coverageHtmlDir = Join-Path $root 'coverage_html'
    New-Item -ItemType Directory -Force -Path $coverageHtmlDir | Out-Null

    gcovr `
        --root . `
        --object-directory (Join-Path $root '.pio/build/native') `
        --filter 'lib/wildlife_core/' `
        --exclude 'test/' `
        --print-summary `
        --decisions `
        --html-details (Join-Path $coverageHtmlDir 'index.html') `
        --xml-pretty --output (Join-Path $root 'coverage.xml') `
        --fail-under-line   $FailUnderLine `
        --fail-under-branch $FailUnderBranch
}
finally {
    Pop-Location
}
