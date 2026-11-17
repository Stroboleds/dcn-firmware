# Espectro DCN v3 — pantalla LVGL (plan)

## v3 — dónde abrirlo (firmware con parche device-ui)

**Ajustes (engranaje) → Herramientas → `DCN Espectro`**

- Sliders **inicio/fin MHz** (850–930, acotado al SX1262)
- **RX boost** y modo **Live** (re-barrido automático)
- Gráfico de barras en pantalla

Requiere build `dcn-t-deck` posterior al commit v3 (parche `patch-device-ui-dcn.py` en CI).

## Histórico — por qué antes no había menú

El T-Deck usa **device-ui** (LVGL). El módulo DCN v1/v2 vive en el **firmware**; la UI hay que extenderla en `meshtastic-device-ui` (Herramientas / panel nuevo).

Herramienta existente **“Signal scanner”** (Ajustes → Herramientas): mide **RSSI de nodos mesh**, no el espectro RF 863–870 MHz.

## API ya en firmware (puente v3)

Tras el fix `loopbackOk`, el firmware exporta:

- `dcn_spectrum_request_sweep(start, end, step)`
- `dcn_spectrum_bin_*` / min / max
- `dcn_spectrum_set_rx_boost(on)`

Cabecera: `overlay/src/modules/optional/DcnSpectrum/DcnSpectrumBridge.h`

## v3 — entrega prevista

1. Parche `scripts/patch-device-ui-dcn.py` en CI (tras `pio pkg install`).
2. Panel **“DCN Espectro”** en Tools:
   - Spinners: centro MHz, anchura, paso
   - Toggle: RX boost (ganancia)
   - Modo **barrido** y **tiempo real** (`lv_chart` + timer 150 ms)
3. Llamadas al puente C desde el hilo LVGL (sin tocar la radio fuera del OSThread del servicio).

## SX1262 vs T-Deck EU

| | MHz |
|---|-----|
| **Chip SX1262** (datasheet) | ~**850–930** (no 700) |
| **Filtro / antena T-Deck EU868** | útil **~855–880**; fuera de banda atenúa |
| **DCN por defecto** | 863–870 (ISM EU) |

“Espectro general 700–950” **no es realista** en este hardware; v3 permitirá elegir rango **dentro de 850–930** con aviso si sales del canal EU.