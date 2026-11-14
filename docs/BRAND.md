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
| Primario | `#0D4F4F` | Meshtastic / celda A |
| Secundario | `#E85D04` | MeshCore / celda B |
| Fondo | `#12151A` | Pantallas oscuras |
| Texto | `#E8EAED` | Cuerpo |
| Acento OK | `#2DD881` | Estado conectado |
| Espectro frío | `#3B82F6` | Ruido |
| Espectro caliente | `#EF4444` | Picos RF |

## Tipografía

- Splash / título: sans condensada en mayúsculas (**DCN**).
- UI embebida: fuentes Meshtastic existentes (no cambiar métricas LVGL sin necesidad).

## Logo (placeholder)

Monograma **DCN** o dos hexágonos entrelazados; exportar luego:

- `assets/dcn_splash_320x48.png` (arranque)
- `assets/dcn_icon_64.png` (about)
- `assets/dcn_icon_512.png` (builder/web)

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