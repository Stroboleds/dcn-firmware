# Analizador de espectro DCN — estado y plan

## Estado actual (honesto)

| Capa | ¿Existe? | Notas |
|------|----------|--------|
| **UI TFT 863–870 MHz** (barras / waterfall en T-Deck) | **v2 (hilo local + █)** | `DCN SPECTRUM` → barrido + gráfico Unicode en mensajes MUI; v3 = LVGL fullscreen |
| **FREQSCAN / NOISE** (meshtacore / Nexus) | **Sí** | Por RF y consola `/mtc`, respuesta en texto |
| **RSSI instantáneo SX126x** | **Sí** | `SX126xInterface::getInstantaneousRSSI()` → RadioLib `getRSSI(false)` |

El objetivo DCN (pantalla 320×240, banda EU ISM, colores del tema del **dispositivo**) **aún no está en el firmware**. Lo que ya puedes usar en un build DCN/meshtacore:

```text
/mtc FREQSCAN 869.5 125
/mtc NOISE
```

Eso muestrea ruido con la radio en RX (≈15 s, TX deshabilitado) y devuelve estadísticas por mensaje — **no** dibuja en la pantalla del T-Deck.

## Cómo lo implementaremos (MVP DCN)

1. **Módulo** `DCNSpectrumService` (solo `DCN_BRAND` + `HAS_TFT` + SX1262):
   - Barrido 863.0–870.0 MHz, paso configurable (p. ej. 25 kHz o por canal SFNarrow).
   - Pausar mesh TX durante el barrido (como FREQSCAN).
2. **UI**: pantalla Meshtastic-device-ui o overlay LVGL simple (espectro + opcional waterfall).
3. **Preset EU**: `overlay/spectrum-eu868.json` con inicio/fin/ancho de banda.
4. **Flag de build**: `-D DCN_SPECTRUM_UI=1` cuando el MVP esté estable.

Estimación: **1–2 sesiones de desarrollo** + prueba en hardware real (la radio no puede TX y espectro a la vez).

## Mientras tanto

1. Compilar **`dcn-t-deck`** en GitHub Actions (tras fix LittleFS en overlay).
2. Probar **FREQSCAN** en la frecuencia de trabajo.
3. Seguir diseño de colores con `overlay/device-ui-themes.json` en el builder (UI del nodo).

## Referencia técnica

- Muestreo Nexus: `NexusModule.cpp` (`noise_sampling_active`, `noise_samples[]`).
- Comando: `NexusCommands.cpp` → `FREQSCAN`.
- API RSSI: `SX126xInterface::getInstantaneousRSSI()`.