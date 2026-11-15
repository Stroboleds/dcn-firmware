#!/usr/bin/env bash
# Aplica overlay DCN al submódulo firmware (Linux/macOS + GitHub Actions).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ ! -d firmware ]]; then
  echo "Falta firmware/. Ejecuta: git submodule update --init --recursive"
  exit 1
fi

if command -v python3 >/dev/null 2>&1; then
  PY=python3
else
  PY=python
fi

if [[ -f branding/exports/logo_320x240.png ]]; then
  :
elif [[ -f scripts/render_dcn_assets.py ]]; then
  "$PY" scripts/render_dcn_assets.py || true
fi

# Boot logo
mkdir -p firmware/branding firmware/data/boot
if [[ -f branding/exports/logo_320x240.png ]]; then
  cp -f branding/exports/logo_320x240.png firmware/branding/logo_320x240.png
  cp -f branding/exports/logo_320x240.png firmware/data/boot/logo.png
fi

# T-Deck overlay
TDECK=firmware/variants/esp32s3/t-deck
mkdir -p "$TDECK"
cp -f overlay/variants/esp32s3/t-deck/sdkconfig.h "$TDECK/sdkconfig.h"
if [[ -f overlay/variants/esp32s3/t-deck/platformio.ini ]]; then
  cp -f overlay/variants/esp32s3/t-deck/platformio.ini "$TDECK/platformio.ini"
fi

"$PY" scripts/merge_userprefs.py
"$PY" scripts/patch-builder-web.py

mkdir -p firmware/images
if [[ -f branding/exports/dcn_icon_64.png ]]; then
  cp -f branding/exports/dcn_icon_64.png firmware/images/dcn_node.png
fi

echo "DCN branding aplicado."