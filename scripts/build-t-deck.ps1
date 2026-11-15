$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$FirmwareDir = Join-Path $Root "firmware"

if (-not (Test-Path $FirmwareDir)) {
    Write-Host "Ejecuta primero: .\scripts\init-firmware.ps1" -ForegroundColor Yellow
    exit 1
}

& (Join-Path (Split-Path -Parent $PSScriptRoot) "scripts\apply-dcn-branding.ps1")

Set-Location $FirmwareDir

$pio = Get-Command pio -ErrorAction SilentlyContinue
if (-not $pio) {
    $pioAlt = "$env:LOCALAPPDATA\Packages\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\LocalCache\local-packages\Python312\Scripts\pio.exe"
    if (Test-Path $pioAlt) { $pio = @{ Source = $pioAlt } } else {
        Write-Host "PlatformIO (pio) no está en PATH. Instala PlatformIO >= 6.2." -ForegroundColor Red
        exit 1
    }
}
& $pio.Source run -e dcn-t-deck