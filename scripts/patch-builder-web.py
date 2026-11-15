#!/usr/bin/env python3
"""Aplica marca DCN/Cell-Mesh al builder web (dentro de firmware/custom_builder_web)."""
from __future__ import annotations

import json
import re
import shutil
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CFG_PATH = ROOT / "overlay" / "dcn-site.json"
WEB = ROOT / "firmware" / "custom_builder_web"
OVERLAY_WEB = ROOT / "overlay" / "custom_builder_web"

HTML_TOUCH = (
    "index.html",
    "builder.html",
    "flasher.html",
    "update.html",
    "admin.html",
    "decrypt.html",
    "docs/index.html",
    "docs/administracion-remota.html",
)

BUILDER_REPLACEMENTS = (
    ("Meshtacore Builder", "Cell-Mesh Builder"),
    ("Web Builder", "Generador Cell-Mesh"),
    ("Meshtacore", "Cell-Mesh"),
    ("meshtacore", "DCN"),
    ("MESHTACORE", "DCN"),
    ("Abrir Meshtacore Builder", "Generar firmware"),
    ("Un Radio, Dos Meshes", "Dual-Cell-Net · Firmware"),
)


def load_cfg() -> dict:
    with CFG_PATH.open(encoding="utf-8") as f:
        return json.load(f)


def patch_js_repo(cfg: dict) -> None:
    owner, name = cfg["repo_owner"], cfg["repo_name"]
    full = f"{owner}/{name}"

    app = WEB / "app.js"
    text = app.read_text(encoding="utf-8")
    text = re.sub(
        r'const REPO_OWNER = "[^"]+";\s*\nconst REPO_NAME = "[^"]+";',
        f'const REPO_OWNER = "{owner}";\nconst REPO_NAME = "{name}";',
        text,
        count=1,
    )
    app.write_text(text, encoding="utf-8")

    env = WEB / "flasher-env.js"
    if not env.is_file():
        raise SystemExit(f"Falta {env}; actualiza el submódulo firmware.")
    text = env.read_text(encoding="utf-8")
    text, n = re.subn(
        r'export const REPO = "[^"]+";',
        f'export const REPO = "{full}";',
        text,
        count=1,
    )
    if n != 1:
        raise SystemExit(f"No pude parchear REPO en {env}")
    env.write_text(text, encoding="utf-8")


def copy_overlay_assets() -> None:
    WEB.mkdir(parents=True, exist_ok=True)
    overlay_index = OVERLAY_WEB / "index.html"
    if overlay_index.is_file():
        shutil.copy2(overlay_index, WEB / "index.html")
    for src in OVERLAY_WEB.iterdir():
        if src.is_file() and src.name != "index.html":
            shutil.copy2(src, WEB / src.name)
    logo = ROOT / "branding" / "source" / "dcn_logo.png"
    if logo.is_file():
        shutil.copy2(logo, WEB / "logo.png")
    for stale in ("dcn-theme.js", "dcn-palette.css"):
        path = WEB / stale
        if path.is_file():
            path.unlink()


def inject_assets(html_path: Path) -> None:
    text = html_path.read_text(encoding="utf-8")
    in_docs = "docs" in html_path.parts
    css_href = "../dcn-brand.css" if in_docs else "dcn-brand.css"
    js_href = "../dcn-footer.js" if in_docs else "dcn-footer.js"

    if "dcn-brand.css" not in text:
        text = text.replace(
            "</head>",
            f'    <link rel="stylesheet" href="{css_href}" />\n'
            f'    <script src="{js_href}" defer></script>\n  </head>',
            1,
        )

    if "dcn-theme.js" in text:
        text = text.replace(
            '<script src="dcn-theme.js"></script>\n', ""
        ).replace('<script src="../dcn-theme.js"></script>\n', "")
    if "dcn-palette.css" in text:
        text = re.sub(
            r'\s*<link rel="stylesheet" href="(\.\./)?dcn-palette\.css" />\s*',
            "\n",
            text,
        )

    html_path.write_text(text, encoding="utf-8")


def patch_page_copy(cfg: dict, path: Path) -> None:
    brand = cfg["brand_name"]
    badge = cfg["brand_badge"]
    text = path.read_text(encoding="utf-8")
    for old, new in BUILDER_REPLACEMENTS:
        text = text.replace(old, new)
    text = text.replace("Dual-Mesh", badge)
    text = re.sub(
        r'<img([^>]+)src="logo\.svg"',
        r'<img\1src="logo.png"',
        text,
    )
    if path.name != "index.html":
        text = re.sub(
            r"(<title>)[^<]+(</title>)",
            rf"\1{brand} (DCN)\2",
            text,
            count=1,
        )
    path.write_text(text, encoding="utf-8")


def sweep_upstream_refs(cfg: dict) -> None:
    owner, name = cfg["repo_owner"], cfg["repo_name"]
    pages = cfg["pages_base"].rstrip("/")
    patterns = [
        ("xejarque/meshtacore-firmware", f"{owner}/{name}"),
        (
            "https://github.com/xejarque/meshtacore-firmware",
            f"https://github.com/{owner}/{name}",
        ),
        ("https://xejarque.github.io/meshtacore-firmware", pages),
        ('const REPO_OWNER = "xejarque"', f'const REPO_OWNER = "{owner}"'),
        (
            'const REPO_NAME = "meshtacore-firmware"',
            f'const REPO_NAME = "{name}"',
        ),
    ]
    for path in WEB.rglob("*"):
        if path.suffix not in {".html", ".js"}:
            continue
        text = path.read_text(encoding="utf-8")
        orig = text
        for old, new in patterns:
            text = text.replace(old, new)
        if text != orig:
            path.write_text(text, encoding="utf-8")


def main() -> None:
    if not WEB.is_dir():
        raise SystemExit(f"No existe {WEB}; inicializa el submódulo firmware.")
    cfg = load_cfg()
    copy_overlay_assets()
    patch_js_repo(cfg)
    for rel in HTML_TOUCH:
        path = WEB / rel
        if not path.is_file():
            continue
        if rel != "index.html":
            inject_assets(path)
            patch_page_copy(cfg, path)
    sweep_upstream_refs(cfg)
    print(
        f"Web DCN → {cfg['repo_owner']}/{cfg['repo_name']} "
        f"(landing propia; temas de color solo en dispositivo)"
    )


if __name__ == "__main__":
    main()