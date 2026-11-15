#!/usr/bin/env python3
"""Patch DCN keys into firmware/userPrefs.jsonc without wiping upstream defaults."""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
FW = ROOT / "firmware"
OVERLAY = ROOT / "overlay" / "userPrefs.dcn.jsonc"
XBM = ROOT / "branding" / "exports" / "dcn_oem.xbm.h"
TARGET = FW / "userPrefs.jsonc"
BACKUP = FW / "userPrefs.jsonc.dcn-backup"


def strip_jsonc_comments(text: str) -> str:
    out: list[str] = []
    i = 0
    in_string = False
    while i < len(text):
        c = text[i]
        if in_string:
            out.append(c)
            if c == "\\" and i + 1 < len(text):
                out.append(text[i + 1])
                i += 2
                continue
            if c == '"':
                in_string = False
            i += 1
            continue
        if c == '"':
            in_string = True
            out.append(c)
            i += 1
            continue
        if c == "/" and i + 1 < len(text) and text[i + 1] == "/":
            while i < len(text) and text[i] != "\n":
                i += 1
            continue
        out.append(c)
        i += 1
    return "".join(out)


def load_jsonc(path: Path) -> dict:
    return json.loads(strip_jsonc_comments(path.read_text(encoding="utf-8")))


def parse_xbm(path: Path) -> tuple[int, int, str]:
    text = path.read_text(encoding="utf-8")
    w = int(re.search(r"DCN_OEM_IMAGE_WIDTH (\d+)", text).group(1))
    h = int(re.search(r"DCN_OEM_IMAGE_HEIGHT (\d+)", text).group(1))
    block = text.split("DCN_OEM_IMAGE_DATA")[1]
    hexes = re.findall(r"0x[0-9a-fA-F]+", block)
    return w, h, "{ " + ", ".join(hexes) + " }"


def format_value(key: str, value: str, trailing_comma: bool = True) -> str:
    suffix = "," if trailing_comma else ""
    if value.startswith("meshtastic_"):
        return f'  "{key}": "{value}"{suffix}'
    if value.startswith("{") and not value.startswith('"{'):
        return f'  "{key}": "{value}"{suffix}'
    return f'  "{key}": {json.dumps(value)}{suffix}'


def patch_key(text: str, key: str, value: str, last: bool = False) -> str:
    line = format_value(key, value, trailing_comma=not last)
    pattern = rf'^\s*"{re.escape(key)}"\s*:\s*.*,?$\n?'
    if re.search(pattern, text, flags=re.MULTILINE):
        return re.sub(pattern, line + "\n", text, count=1, flags=re.MULTILINE)
    insert = line + "\n"
    return text.replace("\n}\n", f"\n{insert}}}\n", 1)


def main() -> None:
    if not TARGET.exists():
        raise SystemExit(f"Missing {TARGET}")

    overlay = load_jsonc(OVERLAY)
    text = TARGET.read_text(encoding="utf-8")
    if not BACKUP.exists():
        BACKUP.write_text(text, encoding="utf-8")

    if XBM.exists():
        w, h, arr = parse_xbm(XBM)
        overlay["USERPREFS_OEM_IMAGE_WIDTH"] = str(w)
        overlay["USERPREFS_OEM_IMAGE_HEIGHT"] = str(h)
        overlay["USERPREFS_OEM_IMAGE_DATA"] = arr

    keys = list(overlay.items())
    for idx, (key, val) in enumerate(keys):
        text = patch_key(text, key, str(val), last=(idx == len(keys) - 1))

    text = re.sub(
        r'("USERPREFS_TZ_STRING": "[^"]*")\s*\n(\s*"USERPREFS_OEM_)',
        r"\1,\n\2",
        text,
    )

    TARGET.write_text(text, encoding="utf-8")
    print(f"Patched {len(overlay)} DCN keys in {TARGET}")


if __name__ == "__main__":
    main()