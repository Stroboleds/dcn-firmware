#!/usr/bin/env python3
"""Sella cada compilación DCN en CI con un número de build distinto (pantalla + zip)."""
from __future__ import annotations

import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VER = ROOT / "firmware" / "version.properties"

def main() -> None:
    if not VER.is_file():
        raise SystemExit(f"No existe {VER}")
    run_id = int(os.environ.get("GITHUB_RUN_ID", "0"))
    text = VER.read_text(encoding="utf-8")
    m = re.search(r"(?m)^build\s*=\s*(\d+)\s*$", text)
    if not m:
        raise SystemExit("No se encontró build = en version.properties")
    base = int(m.group(1))
    # Últimos 3 dígitos del run id → sube casi en cada compilación sin números enormes.
    ci_build = base + (run_id % 900)
    text = re.sub(
        r"(?m)^build\s*=\s*\d+\s*$",
        f"build = {ci_build}",
        text,
        count=1,
    )
    VER.write_text(text, encoding="utf-8")
    print(f"DCN CI build stamp: {ci_build} (base {base}, run_id {run_id})")


if __name__ == "__main__":
    main()