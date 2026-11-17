# Dual-Cell-Net (DCN) — Firmware

Firmware y documentación para **Dual-Cell-Net (DCN)**: marca propia sobre base **Meshtastic + MeshCore** ([meshtacore-firmware](https://github.com/xejarque/meshtacore-firmware)), orientado al **LILYGO T-Deck Plus** (ESP32-S3 + SX1262 **868 MHz**), con soporte multi-protocolo y **analizador de espectro** en banda EU ISM.

> **DCN** = dos redes mesh (“celdas”), una radio SX1262.

## Estado

| Fase | Descripción |
|------|-------------|
| 🟢 Listo | CI en GitHub (`custom_builder.yml`, `pages.yml`) + builder web Cell-Mesh |
| 🟢 Listo | Build `dcn-t-deck` EU868 en Actions + release `dcn-latest` |
| 🟢 v2 | Espectro: comando `DCN SPECTRUM` + hilo local (validado) |
| 🟡 v3 | Pantalla LVGL **Ajustes → Herramientas → DCN Espectro** (tiempo real, MHz, boost) |

## Requisitos de hardware

| Componente | Especificación |
|------------|----------------|
| Dispositivo | LILYGO **T-Deck Plus** |
| MCU | ESP32-S3 |
| Radio | **SX1262** variante **868 MHz** (EU) |
| Pantalla | 320×240 (UI Meshtastic) |

## Estructura del repositorio

```
.
├── README.md                 # Este archivo
├── docs/
│   ├── BRAND.md              # Guía de marca DCN (colores, textos)
│   └── BUILD-T-DECK-PLUS.md  # Compilar y flashear
├── scripts/
│   ├── init-firmware.ps1     # Clonar/submodule upstream
│   └── build-t-deck.ps1      # Atajo de build PlatformIO
└── firmware/                 # Árbol meshtacore (submodule), tras init
```

## Inicio rápido (Windows)

### 1. Obtener el árbol de firmware

```powershell
cd c:\Users\Lenovo\Documents\dcn-firmware
.\scripts\init-firmware.ps1
```

### 2. Compilar para T-Deck (EU868)

```powershell
.\scripts\build-t-deck.ps1
```

Requiere [PlatformIO](https://platformio.org/) (extensión VS Code/Cursor o CLI).

### 3. Flashear

Con el T-Deck Plus en USB:

```powershell
cd firmware
pio run -e t-deck -t upload --upload-port COMx
```

Sustituye `COMx` por el puerto serie correcto.

## Roadmap técnico

1. **Branding DCN** — splash, “About”, metadatos de build (sin eliminar atribución Meshtastic® / MeshCore).
2. **Meshtastic + MeshCore** — heredar comportamiento meshtacore en 868 MHz.
3. **Analizador de espectro** — barrido SX1262 (`GET_RSSI_INST`), preset **863–870 MHz**, modos espectro/waterfall en pantalla T-Deck.
4. **Builder / releases** — ver [docs/GITHUB-BUILDER.md](docs/GITHUB-BUILDER.md) (Pages + compilación por usuario).

> **Repo público solo DCN/Cell-Mesh.** Proyectos no relacionados (p. ej. TETRA) no van aquí.

## Licencia y marcas

- El firmware upstream es **GPL-3.0** (Meshtastic / meshtacore). Las modificaciones DCN deben publicarse bajo la misma licencia si se distribuyen binarios.
- **Meshtastic®** es marca registrada de Meshtastic LLC. DCN no está afiliado ni respaldado por Meshtastic ni MeshCore.
- Ver créditos en [meshtacore README](https://github.com/xejarque/meshtacore-firmware/blob/main/README.md).

## Enlaces

- Upstream: https://github.com/xejarque/meshtacore-firmware
- Meshtastic docs: https://meshtastic.org/docs/
- T-Deck Plus: https://meshtastic.org/docs/hardware/devices/lilygo/

---

**Repositorio privado** — configuración de equipo igual que el proyecto TETRA BTS (scripts, documentación, despliegue controlado).