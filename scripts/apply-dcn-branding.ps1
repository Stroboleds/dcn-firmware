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
$bootDir = Join-Path $Fw "data\boot"
New-Item -ItemType Directory -Force -Path $bootDir | Out-Null
Copy-Item (Join-Path $brandDir "logo_320x240.png") (Join-Path $bootDir "logo.png") -Force

# Windows: cp/rm en load_boot_logo sustituido por shutil (parche DCN en bin/platformio-custom.py)
$pioCustom = Join-Path $Fw "bin\platformio-custom.py"
$pioBody = Get-Content $pioCustom -Raw
if ($pioBody -match 'env\.Execute\(f"cp ') {
    $pioBody = $pioBody -replace '(?s)    if env\.File\(source_path\)\.exists\(\):\s+print\(f"Loading boot logo from \{source_path\}"\).*?env\.Execute\(f"cp \{source_path\} \{dest_path\}"\)', @'
    if env.File(source_path).exists():
        print(f"Loading boot logo from {source_path}")
        import os
        import shutil

        os.makedirs(dest_dir, exist_ok=True)
        if os.path.exists(dest_path):
            os.remove(dest_path)
        shutil.copy2(source_path, dest_path)
'@
    Set-Content -Path $pioCustom -Value $pioBody -NoNewline
}

# Web builder logo
$builderSvg = Join-Path (Join-Path (Join-Path $Root "branding") "source") "dcn_builder.svg"
$builderDst = Join-Path (Join-Path $Fw "custom_builder_web") "logo.svg"
if (Test-Path $builderSvg) {
    Copy-Item $builderSvg $builderDst -Force
}

# PlatformIO env DCN (must live in platformio.ini — extra_configs ignores other names)
$tdeckDir = Join-Path (Join-Path (Join-Path (Join-Path $Fw "variants") "esp32s3") "t-deck")
Copy-Item (Join-Path (Join-Path (Join-Path (Join-Path $Root "overlay") "variants") "esp32s3") "t-deck") "sdkconfig.h") (Join-Path $tdeckDir "sdkconfig.h") -Force

$pioOverlay = Join-Path (Join-Path (Join-Path (Join-Path (Join-Path $Root "overlay") "variants") "esp32s3") "t-deck") "platformio.ini"
$pioBase = Join-Path (Join-Path (Join-Path $Fw "variants") "esp32s3") "t-deck\platformio.ini"
if (Test-Path $pioOverlay) {
    Copy-Item $pioOverlay $pioBase -Force
}

# Merge userPrefs: overlay keys + generate OEM XBM from render output
$mergePy = Join-Path (Join-Path $Root "scripts") "merge_userprefs.py"
python $mergePy

# Map node icon placeholder (app)
$imgDst = Join-Path $Fw "images"
New-Item -ItemType Directory -Force -Path $imgDst | Out-Null
Copy-Item (Join-Path $Exports "dcn_icon_64.png") (Join-Path $imgDst "dcn_node.png") -Force

$logoSrc = Join-Path (Join-Path (Join-Path $Root "branding") "source") "dcn_logo.png"
$webDir = Join-Path $Fw "custom_builder_web"
if (Test-Path $logoSrc) {
    Copy-Item $logoSrc (Join-Path $webDir "logo.png") -Force
}

python (Join-Path (Join-Path $Root "scripts") "patch-builder-web.py")
python (Join-Path (Join-Path $Root "scripts") "patch-firmware-dcn.py")

$optSrc = Join-Path (Join-Path (Join-Path $Root "overlay") "src") "modules\optional"
$optDst = Join-Path (Join-Path $Fw "src") "modules\optional"
if (Test-Path $optSrc) {
    New-Item -ItemType Directory -Force -Path $optDst | Out-Null
    Copy-Item (Join-Path $optSrc "*") $optDst -Recurse -Force
}

Write-Host "DCN branding applied under firmware/ (submodule cambiado localmente)." -ForegroundColor Green
Write-Host "Compila con: pio run -e dcn-t-deck  (desde firmware/)" -ForegroundColor Cyan