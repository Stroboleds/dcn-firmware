# Guía de marca — Dual-Cell-Net (DCN)

Documento de referencia hasta disponer de logos finales.

## Nombre

| Uso | Texto |
|-----|--------|
| Nombre completo | Dual-Cell-Net |
| Corto | DCN |
| Claim (ES) | Dos redes, una radio. |
| Claim (EN) | Two cells, one antenna. |

## Concepto visual

Dos “celdas” (mallas) compartiendo un núcleo: alinea con Meshtastic + MeshCore en un solo SX1262.

## Paleta (UI 320×240 y web)

| Rol | Hex | Uso |
|-----|-----|-----|
| Primario UI web (defecto) | `#0077FF` | Azul eléctrico — paleta builder |
| Primario DCN | `#0D4F4F` | Meshtastic / celda A |
| Secundario | `#E85D04` | MeshCore / celda B |
| Fondo | `#12151A` | Pantallas oscuras |
| Texto | `#E8EAED` | Cuerpo |
| Acento OK | `#2DD881` | Estado conectado |
| Espectro frío | `#3B82F6` | Ruido |
| Espectro caliente | `#EF4444` | Picos RF |

## Tipografía

- Splash / título: sans condensada en mayúsculas (**DCN**).
- UI embebida: fuentes Meshtastic existentes (no cambiar métricas LVGL sin necesidad).

## Logo (implementado)

Fuente: `branding/source/`. Export: `branding/exports/` (script `render_dcn_assets.py`).

| Archivo | Uso |
|---------|-----|
| `logo_320x240.png` | Arranque T-Deck TFT |
| `dcn_icon_512.png` | GitHub / releases |
| `dcn_icon_64.png` | About / mapa (`firmware/images/dcn_node.png`) |
| `dcn_builder.svg` | Builder web |
| `dcn_oem.xbm.h` | OEM en `userPrefs` |

## Textos legales en firmware

En pantalla “Acerca de”, incluir siempre:

```
Dual-Cell-Net (DCN)
Basado en Meshtastic firmware y meshtacore (MeshCore bridge).
Meshtastic® es marca de Meshtastic LLC.
```

## Canales por defecto (propuesta EU)

Alinear con política de la comunidad; no sustituir PSK sin acuerdo del equipo:

- Meshtastic: región **EU868**, canales según despliegue DCN.
- MeshCore: canales `MC:*` según meshtacore.