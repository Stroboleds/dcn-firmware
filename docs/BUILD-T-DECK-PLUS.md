# Build — T-Deck Plus (868 MHz)

## Entorno PlatformIO

Tras `scripts\init-firmware.ps1`, el directorio `firmware/` contiene meshtacore.

Entorno habitual para T-Deck:

```bash
pio run -e t-deck
```

Variantes relacionadas en upstream: `t-deck`, `t-deck-pro`, `t-deck-pro-v1_1`. El **T-Deck Plus** suele compilarse como **`t-deck`** con región EU en la app/cliente; confirmar en `firmware/variants/esp32s3/t-deck/`.

## Region EU868

Configurar en el cliente Meshtastic o en `userPrefs.json` / canales de fábrica según builder DCN:

- Frecuencia por defecto dentro de **863–870 MHz** (EU ISM).

## Analizador de espectro (planeado)

- Barrido base: **863.0–870.0 MHz**, paso 250–500 kHz.
- API: RSSI instantáneo SX126x vía RadioLib / comando `GET_RSSI_INST`.
- Al salir del analizador, restaurar frecuencia y modo RX del mesh.

## Problemas frecuentes

| Síntoma | Acción |
|---------|--------|
| `pio` no encontrado | Instalar PlatformIO CLI o extensión |
| Puerto incorrecto | Administrador de dispositivos → COM |
| Build enorme primera vez | Normal; descarga toolchains |