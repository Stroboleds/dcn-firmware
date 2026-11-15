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

HTML_NAMES = (
    "index.html",
    "builder.html",
    "flasher.html",
    "update.html",
    "admin.html",
    "decrypt.html",
    "docs/index.html",
    "docs/administracion-remota.html",
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
    env.write_text(
        f'''// Repositorio DCN ({full})
export const REPO = "{full}";
''',
        encoding="utf-8",
    )

    for path in WEB.rglob("flasher-github.js"):
        # sin cambios estructurales
        pass


def copy_overlay_assets() -> None:
    WEB.mkdir(parents=True, exist_ok=True)
    if OVERLAY_WEB.is_dir():
        for src in OVERLAY_WEB.iterdir():
            if src.is_file():
                shutil.copy2(src, WEB / src.name)
    logo = ROOT / "branding" / "source" / "dcn_logo.png"
    if logo.is_file():
        shutil.copy2(logo, WEB / "logo.png")


def inject_assets(html_path: Path) -> None:
    text = html_path.read_text(encoding="utf-8")
    in_docs = "docs" in html_path.parts
    css_href = "../dcn-palette.css" if in_docs else "dcn-palette.css"
    js_href = "../dcn-theme.js" if in_docs else "dcn-theme.js"
    if "dcn-palette.css" not in text:
        text = text.replace(
            "</head>",
            f'    <link rel="stylesheet" href="{css_href}" />\n  </head>',
            1,
        )
    if "dcn-theme.js" not in text:
        if '<script src="../theme.js"></script>' in text:
            text = text.replace(
                '<script src="../theme.js"></script>',
                f'<script src="../theme.js"></script>\n    <script src="{js_href}"></script>',
                1,
            )
        else:
            text = text.replace(
                '<script src="theme.js"></script>',
                f'<script src="theme.js"></script>\n    <script src="{js_href}"></script>',
                1,
            )
    html_path.write_text(text, encoding="utf-8")


def rebrand_html(cfg: dict) -> None:
    owner, name = cfg["repo_owner"], cfg["repo_name"]
    gh = f"https://github.com/{owner}/{name}"
    brand = cfg["brand_name"]
    short = cfg["brand_short"]
    badge = cfg["brand_badge"]
    pages = cfg["pages_base"]

    replacements = [
        ("https://github.com/xejarque/meshtacore-firmware", gh),
        ("xejarque/meshtacore-firmware", f"{owner}/{name}"),
        ("https://xejarque.github.io/meshtacore-firmware", pages.rstrip("/")),
        ("Meshtacore Logo", f"{brand} logo"),
        (">Meshtacore<", f">{brand}<"),
        ("Meshtacore -", f"{brand} (DCN) -"),
        ("Meshtacore es un firmware", f"{brand} (DCN) es un firmware"),
        ("Dual-Mesh", badge),
        ("meshtacore-firmware", name),
    ]

    for rel in HTML_NAMES:
        path = WEB / rel
        if not path.is_file():
            continue
        inject_assets(path)
        text = path.read_text(encoding="utf-8")
        for old, new in replacements:
            text = text.replace(old, new)
        text = re.sub(
            r'<img([^>]+)src="logo\.svg"',
            r'<img\1src="logo.png"',
            text,
            count=0,
        )
        text = re.sub(
            r'content="Meshtacore[^"]*"',
            f'content="{brand} — firmware dual-mesh LoRa (DCN)"',
            text,
            count=1,
        )
        path.write_text(text, encoding="utf-8")


def sweep_upstream_refs(cfg: dict) -> None:
    owner, name = cfg["repo_owner"], cfg["repo_name"]
    pages = cfg["pages_base"].rstrip("/")
    patterns = [
        ("xejarque/meshtacore-firmware", f"{owner}/{name}"),
        ("https://github.com/xejarque/meshtacore-firmware", f"https://github.com/{owner}/{name}"),
        ("https://xejarque.github.io/meshtacore-firmware", pages),
        ('const REPO_OWNER = "xejarque"', f'const REPO_OWNER = "{owner}"'),
        ('const REPO_NAME = "meshtacore-firmware"', f'const REPO_NAME = "{name}"'),
    ]
    for path in WEB.rglob("*"):
        if path.suffix not in {".html", ".js", ".md"}:
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
    rebrand_html(cfg)
    sweep_upstream_refs(cfg)
    print(
        f"Builder web parcheado → {cfg['repo_owner']}/{cfg['repo_name']} ({cfg['brand_name']})"
    )


if __name__ == "__main__":
    main()