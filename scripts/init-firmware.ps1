# Inicializa el submodule o clone de meshtacore-firmware en ./firmware
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$FirmwareDir = Join-Path $Root "firmware"
$Upstream = "https://github.com/xejarque/meshtacore-firmware.git"

Set-Location $Root

if (Test-Path (Join-Path $FirmwareDir ".git")) {
    Write-Host "firmware/ ya existe. Actualizando..." -ForegroundColor Cyan
    Set-Location $FirmwareDir
    git fetch --depth 1 origin main 2>$null
    git pull origin main
    Set-Location $Root
    exit 0
}

if (Test-Path (Join-Path $Root ".git")) {
    Write-Host "Añadiendo submodule firmware..." -ForegroundColor Cyan
    git submodule add --depth 1 $Upstream firmware
    git submodule update --init --depth 1 firmware
} else {
    Write-Host "Clonando upstream (shallow) en firmware/..." -ForegroundColor Cyan
    git clone --depth 1 $Upstream $FirmwareDir
}

Write-Host "Listo. Siguiente: .\scripts\build-t-deck.ps1" -ForegroundColor Green