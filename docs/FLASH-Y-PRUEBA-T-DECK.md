# T-Deck Plus — flasheo y prueba DCN (A)

Hardware: **LILYGO T-Deck Plus**, SX1262 **EU868**, USB **COM8** (ajústalo si cambia).

## 1. Descargar firmware

- Enlace fijo: [dcn-dcn-t-deck-latest.zip](https://github.com/Stroboleds/dcn-firmware/releases/download/dcn-latest/dcn-dcn-t-deck-latest.zip)  
- O el zip más reciente en [Release `dcn-latest`](https://github.com/Stroboleds/dcn-firmware/releases/tag/dcn-latest).

Tras compilar en CI, la versión en pantalla debería ser **2.8.2xx** (no siempre **201**).

## 2. Flashear (recomendado)

### Opción web (con LittleFS)

1. Chrome/Edge: https://stroboleds.github.io/dcn-firmware/flasher.html (`Ctrl+F5`)
2. Elige el `.zip` completo.
3. Marca **Borrar toda la flash antes** (viene marcado por defecto).
4. Conecta USB → **Conectar y flashear** → puerto COM.

### Opción script (más fiable si el nodo no arranca)

Descomprime el zip y ejecuta:

```bat
set ESPTOOL_PORT=COM8
device-install.bat -f firmware-dcn-t-deck-*.factory.bin
```

Escribe **factory + OTA loader + LittleFS** (lo que el flasher web omitía antes).

## 3. Comprobar arranque

- Tras el flash, espera ~30 s; pulsa **RESET** si la pantalla sigue negra.
- En **Ajustes → Información** (o pie de pantalla): versión **2.8.2xx** y entorno `dcn-t-deck`.

## 4. Probar espectro (comando B — v2)

1. Abre **Mensajes** (canal primario o mensaje a ti mismo).
2. Envía texto: `DCN SPECTRUM` (mayúsculas/minúsculas da igual).
3. La pantalla debería:
   - Despertar y mostrar progreso (`DCN Spectrum N/M`).
   - Al terminar, un **hilo local** con barras **█** por frecuencia (863–870 MHz).
4. También llega un resumen por mesh (texto `#` ASCII) en el mismo canal.

Si la radio está ocupada, responde: `DCN: radio ocupada…`.

## 5. Si sigue sin arrancar

En otra terminal (Python + esptool):

```bat
python -m esptool --port COM8 read_mac
```

Comparte el log serial a **115200** justo tras RESET (primeros segundos).