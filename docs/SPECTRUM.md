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

## Dónde está en el T-Deck

| Qué | Dónde |
|-----|--------|
| **Comando DCN** | Mensajes → enviar `DCN SPECTRUM` o `dcn spectrum 865 875` (también desde el teclado del T-Deck tras fix **loopbackOk**) |
| **Ganancia RX** | `dcn gain on` / `dcn gain off` (equivale a *boosted gain* SX126x) |
| **Espectro en pantalla** | v2: hilo local con barras █; **v3**: menú LVGL → [DEVICE-UI-SPECTRUM-V3.md](DEVICE-UI-SPECTRUM-V3.md) |
| **Signal scanner (Meshtastic)** | Ajustes → Herramientas → **no es** espectro RF DCN |

## SX1262 — ¿700–950 MHz?

No. El **SX1262** trabaja aprox. en **850–930 MHz** (Lora sub-GHz). **700 MHz no** lo cubre este chip.

El **T-Deck Plus EU868** lleva filtro/antena para **868 MHz**: podrás sintonizar más ancho en software (hasta ~930), pero con mucha atenuación fuera de la banda EU.

## Mientras tanto

1. Compilar **`dcn-t-deck`** con build reciente (módulo + `loopbackOk`).
2. Opcional upstream: `/mtc FREQSCAN` / `NOISE` por consola.
3. v3 LVGL: ver [DEVICE-UI-SPECTRUM-V3.md](DEVICE-UI-SPECTRUM-V3.md).

## Referencia técnica

- Muestreo Nexus: `NexusModule.cpp` (`noise_sampling_active`, `noise_samples[]`).
- Comando: `NexusCommands.cpp` → `FREQSCAN`.
- API RSSI: `SX126xInterface::getInstantaneousRSSI()`.