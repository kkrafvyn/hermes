# ESP32-S3 flash helper (requires ESP-IDF in PATH)
param(
    [string]$Port = "COM3",
    [switch]$Monitor
)

$ErrorActionPreference = "Stop"
$Project = Join-Path $PSScriptRoot "hardware\esp32-s3"

if (-not (Get-Command idf.py -ErrorAction SilentlyContinue)) {
    Write-Host "ESP-IDF not in PATH."
    Write-Host "Open 'ESP-IDF PowerShell' from Start Menu, then run:"
    Write-Host "  .\scripts\flash_esp32.ps1 -Port COM3"
    exit 1
}

Push-Location $Project
idf.py set-target esp32s3
idf.py build
if ($Monitor) {
    idf.py -p $Port flash monitor
} else {
    idf.py -p $Port flash
}
Pop-Location
