$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$FirmwareDir = Join-Path $Root "firmware"

if (-not (Test-Path $FirmwareDir)) {
    Write-Host "Ejecuta primero: .\scripts\init-firmware.ps1" -ForegroundColor Yellow
    exit 1
}

Set-Location $FirmwareDir

if (-not (Get-Command pio -ErrorAction SilentlyContinue)) {
    Write-Host "PlatformIO (pio) no está en PATH." -ForegroundColor Red
    exit 1
}

pio run -e t-deck