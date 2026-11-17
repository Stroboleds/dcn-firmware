#!/usr/bin/env python3
"""Copia panel DCN Espectro (v3) al meshtastic-device-ui y parchea TFTView."""
from __future__ import annotations

import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OVERLAY_DCN = ROOT / "overlay" / "device-ui" / "dcn"
VIEW_CPP = "source/graphics/TFT/TFTView_320x240.cpp"


def find_device_ui(firmware_dir: Path) -> Path | None:
    libdeps = firmware_dir / ".pio" / "libdeps"
    if not libdeps.is_dir():
        return None
    for child in libdeps.iterdir():
        if not child.is_dir():
            continue
        candidate = child / "meshtastic-device-ui"
        if candidate.is_dir():
            return candidate
    for candidate in firmware_dir.rglob("meshtastic-device-ui"):
        if (candidate / "library.json").is_file():
            return candidate
    return None


def patch_tft_header(device_ui: Path) -> None:
    header = device_ui / "include/graphics/view/TFT/TFTView_320x240.h"
    text = header.read_text(encoding="utf-8")
    # Quitar declaraciones de intentos anteriores (quedaban en protected).
    text = re.sub(
        r"\n#ifdef DCN_BRAND\n    void dcnSpectrumShowPanel\(lv_obj_t \*b, lv_obj_t \*p, lv_obj_t \*tp\);\n#endif\n",
        "",
        text,
    )
    text = re.sub(
        r"\n    void dcnSpectrumShowPanel\(lv_obj_t \*b, lv_obj_t \*p, lv_obj_t \*tp\);\n",
        "",
        text,
        count=1,
    )
    anchor = "  protected:\n    struct NodeFilter {"
    if anchor in text and "dcnSpectrumShowPanel" not in text:
        text = text.replace(
            anchor,
            "#ifdef DCN_BRAND\n"
            "    void dcnSpectrumShowPanel(lv_obj_t *b, lv_obj_t *p, lv_obj_t *tp);\n"
            "#endif\n\n"
            "  protected:\n    struct NodeFilter {",
            1,
        )
        header.write_text(text, encoding="utf-8")
        print(f"Parcheado {header}")


def patch_tft_view(device_ui: Path) -> None:
    cpp = device_ui / VIEW_CPP
    if not cpp.is_file():
        raise SystemExit(f"Falta {cpp}")
    text = cpp.read_text(encoding="utf-8")
    inc = '#include "DcnSpectrumPanel.h"\n'
    if "DcnSpectrumPanel.h" not in text:
        anchor = '#include "graphics/view/TFT/TFTView_320x240.h"'
        if anchor not in text:
            anchor = '#include "TFTView_320x240.h"'
        text = text.replace(anchor, anchor + "\n" + inc, 1)

    hook = "\n#ifdef DCN_BRAND\n    dcnSpectrumPanelInstall(this);\n#endif"
    if "dcnSpectrumPanelInstall" not in text:
        marker = "screensInitialised = true;"
        if marker not in text:
            raise SystemExit("No se encontró screensInitialised en TFTView")
        text = text.replace(marker, marker + hook, 1)

    if "dcnSpectrumShowPanel" not in text:
        stub = (
            "\n#ifdef DCN_BRAND\n"
            "void TFTView_320x240::dcnSpectrumShowPanel(lv_obj_t *b, lv_obj_t *p, lv_obj_t *tp)\n"
            "{\n    ui_set_active(b, p, tp);\n}\n#endif\n"
        )
        text = text + stub

    cpp.write_text(text, encoding="utf-8")
    print(f"Parcheado {cpp}")


def main() -> None:
    firmware = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "firmware"
    ui = find_device_ui(firmware)
    if not ui:
        print("patch-device-ui-dcn: meshtastic-device-ui aún no instalado; se omite")
        return
    dest = ui / "source" / "dcn"
    dest.mkdir(parents=True, exist_ok=True)
    if not OVERLAY_DCN.is_dir():
        raise SystemExit(f"Falta {OVERLAY_DCN}")
    for src in OVERLAY_DCN.glob("*"):
        if src.is_file():
            shutil.copy2(src, dest / src.name)
    patch_tft_header(ui)
    patch_tft_view(ui)
    es = ui / "extra_script.py"
    es_orig = es.read_text(encoding="utf-8")
    es_text = es_orig
    if '"+<source/dcn>"' not in es_text:
        es_text = es_text.replace('"+<source>"', '"+<source>", "+<source/dcn>"')
    if 'source/dcn")' not in es_text:
        es_text = es_text.replace(
            "env.Replace(SRC_FILTER=src_filter)",
            'env.Append(CPPPATH=[realpath("source/dcn")])\nenv.Replace(SRC_FILTER=src_filter)',
            1,
        )
    if es_text != es_orig:
        es.write_text(es_text, encoding="utf-8")
    print(f"DCN device-ui v3 → {ui}")


if __name__ == "__main__":
    main()