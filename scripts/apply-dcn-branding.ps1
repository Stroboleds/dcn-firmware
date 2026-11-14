# Copy DCN branding into meshtacore submodule (local overlay; re-run after submodule update)
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Fw = Join-Path $Root "firmware"
$Exports = Join-Path (Join-Path $Root "branding") "exports"

if (-not (Test-Path $Fw)) {
    Write-Host "Falta firmware/. Ejecuta .\scripts\init-firmware.ps1" -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $Exports)) {
    Write-Host "Generando assets PNG..." -ForegroundColor Cyan
    python (Join-Path (Join-Path $Root "scripts") "render_dcn_assets.py")
}

# Boot splash (TFT 320x240)
$brandDir = Join-Path $Fw "branding"
New-Item -ItemType Directory -Force -Path $brandDir | Out-Null
Copy-Item (Join-Path $Exports "logo_320x240.png") (Join-Path $brandDir "logo_320x240.png") -Force

# Web builder logo
$builderSvg = Join-Path (Join-Path (Join-Path $Root "branding") "source") "dcn_builder.svg"
$builderDst = Join-Path (Join-Path $Fw "custom_builder_web") "logo.svg"
if (Test-Path $builderSvg) {
    Copy-Item $builderSvg $builderDst -Force
}

# PlatformIO env DCN
$pioSrc = Join-Path (Join-Path (Join-Path (Join-Path (Join-Path $Root "overlay") "variants") "esp32s3") "t-deck") "platformio_dcn.ini"
$pioDst = Join-Path (Join-Path (Join-Path (Join-Path $Fw "variants") "esp32s3") "t-deck") "platformio_dcn.ini"
Copy-Item $pioSrc $pioDst -Force

# T-Deck display name in base env (fallback)
$pioBase = Join-Path (Join-Path (Join-Path $Fw "variants") "esp32s3") "t-deck\platformio.ini"
$content = Get-Content $pioBase -Raw
if ($content -notmatch "Dual-Cell-Net") {
    $content = $content -replace 'custom_meshtastic_display_name = LILYGO T-Deck', 'custom_meshtastic_display_name = Dual-Cell-Net T-Deck'
    Set-Content -Path $pioBase -Value $content -NoNewline
}

# Merge userPrefs: overlay keys + generate OEM XBM from render output
$mergePy = Join-Path (Join-Path $Root "scripts") "merge_userprefs.py"
python $mergePy

# Map node icon placeholder (app)
$imgDst = Join-Path $Fw "images"
New-Item -ItemType Directory -Force -Path $imgDst | Out-Null
Copy-Item (Join-Path $Exports "dcn_icon_64.png") (Join-Path $imgDst "dcn_node.png") -Force

Write-Host "DCN branding applied under firmware/ (submodule cambiado localmente)." -ForegroundColor Green
Write-Host "Compila con: pio run -e dcn-t-deck  (desde firmware/)" -ForegroundColor Cyan