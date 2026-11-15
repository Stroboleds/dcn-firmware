# Cell-Mesh / DCN — compilación en GitHub y builder web

Este repositorio **es público** y contiene solo **Dual-Cell-Net (DCN) / Cell-Mesh**. No incluye proyectos privados (p. ej. TETRA).

## Qué obtienes

| Servicio | URL (tras activar Pages) |
|----------|---------------------------|
| Landing + builder | `https://stroboleds.github.io/dcn-firmware/` |
| Web Builder | `…/builder.html` |
| Flasher USB | `…/flasher.html` |

## Activar GitHub Pages (una vez)

1. Repo **público** `Stroboleds/dcn-firmware`.
2. **Settings → Pages → Build and deployment**: origen **GitHub Actions**.
3. Push a `main` (o ejecutar workflow **Deploy Cell-Mesh / DCN Builder**).

## Compilación por usuario

Cada usuario elige placa, región **EU_868**, canales y nombre de nodo en el builder. La compilación corre en **GitHub Actions** (`custom_builder.yml`).

1. Abrir `builder.html` en Pages.
2. Configurar el firmware (T-Deck → `t-deck-tft` o entorno **dcn-t-deck** si aparece en el dispatch manual).
3. Disparar build con token GitHub (`repo` + lectura de Actions) o vía issue `[Build Request]` (límite ~6/día por usuario, como upstream).

El repo del builder apunta a **`Stroboleds/dcn-firmware`** (ver `overlay/dcn-site.json`).

## Temas y paletas

En la barra superior: **modo claro/oscuro** y selector **Tema** (paleta de acento).

| Paleta | Uso |
|--------|-----|
| **Azul eléctrico** | Por defecto (sustituye el verde meshtacore) |
| DCN teal / Cell naranja / Violeta | Alternativas |
| Verde clásico | Estilo upstream |

Preferencias en `localStorage` (`dcn_palette`, `dcn_theme_mode`).

## Branding local antes de compilar

```bash
git submodule update --init --recursive
bash scripts/apply-dcn-branding.sh
```

Windows: `.\scripts\apply-dcn-branding.ps1`

CI ejecuta el mismo script en cada job.

## Licencia

Firmware bajo **GPL-3.0** (upstream Meshtastic / meshtacore). Conservar créditos y atribución Meshtastic®.