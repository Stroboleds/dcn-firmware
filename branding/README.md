# DCN branding assets

| Path | Uso |
|------|-----|
| `source/dcn_icon.svg` | Icono maestro (mapa, favicon) |
| `source/dcn_builder.svg` | Logo del web builder meshtacore → DCN |
| `exports/logo_320x240.png` | Boot TFT T-Deck Plus |
| `exports/dcn_icon_512.png` | GitHub / releases |
| `exports/dcn_icon_64.png` | UI / nodo |
| `exports/dcn_oem.xbm.h` | OEM XBM (USERPREFS) |

Regenerar PNG/XBM:

```powershell
python scripts/render_dcn_assets.py
```

Aplicar al submodule:

```powershell
.\scripts\apply-dcn-branding.ps1
```