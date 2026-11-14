#!/usr/bin/env python3
"""Render DCN PNG assets from vector-style drawing (Pillow)."""
from __future__ import annotations

import math
from pathlib import Path

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    raise SystemExit("pip install pillow")

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "branding" / "exports"

BG = (0x12, 0x15, 0x1A)
TEAL = (0x0D, 0x4F, 0x4F)
TEAL2 = (0x1A, 0x7A, 0x7A)
AMBER = (0xE8, 0x5D, 0x04)
GREEN = (0x2D, 0xD8, 0x81)
TEXT = (0xE8, 0xEA, 0xED)


def _font(size: int):
    for name in ("segoeui.ttf", "arial.ttf", "DejaVuSans-Bold.ttf"):
        try:
            return ImageFont.truetype(name, size)
        except OSError:
            continue
    return ImageFont.load_default()


def draw_dcn_mark(draw: ImageDraw.ImageDraw, cx: int, cy: int, scale: float) -> None:
    """Dual-cell logo: two ellipses + hexagon."""
    rx, ry = int(42 * scale), int(18 * scale)
    for angle, color in ((-32, TEAL2), (32, AMBER)):
        rad = math.radians(angle)
        # Approximate rotated ellipse with polygon
        pts = []
        for t in range(0, 360, 12):
            th = math.radians(t)
            x = rx * math.cos(th)
            y = ry * math.sin(th)
            xr = x * math.cos(rad) - y * math.sin(rad)
            yr = x * math.sin(rad) + y * math.cos(rad)
            pts.append((cx + xr, cy + yr))
        draw.polygon(pts, outline=color, width=max(1, int(3 * scale)))

    r = 4 * scale
    draw.ellipse((cx - 24 * scale - r, cy - 18 * scale - r, cx - 24 * scale + r, cy - 18 * scale + r), fill=GREEN)
    draw.ellipse((cx + 24 * scale - r, cy + 18 * scale - r, cx + 24 * scale + r, cy + 18 * scale + r), fill=AMBER)

    hs = 14 * scale
    hex_pts = []
    for i in range(6):
        a = math.radians(60 * i - 30)
        hex_pts.append((cx + hs * math.cos(a), cy + hs * 0.9 * math.sin(a)))
    draw.polygon(hex_pts, fill=TEAL, outline=TEXT, width=max(1, int(1.5 * scale)))


def render_icon(size: int) -> Image.Image:
    img = Image.new("RGB", (size, size), BG)
    draw = ImageDraw.Draw(img)
    draw_dcn_mark(draw, size // 2, int(size * 0.42), size / 120)
    font = _font(max(10, size // 8))
    label = "DCN" if size >= 96 else ""
    if label:
        bbox = draw.textbbox((0, 0), label, font=font)
        tw = bbox[2] - bbox[0]
        draw.text(((size - tw) // 2, int(size * 0.78)), label, fill=TEXT, font=font)
    return img


def render_splash() -> Image.Image:
    w, h = 320, 240
    img = Image.new("RGB", (w, h), BG)
    draw = ImageDraw.Draw(img)
    draw_dcn_mark(draw, w // 2, 95, 2.0)

    title_font = _font(28)
    sub_font = _font(14)
    title = "Dual-Cell-Net"
    sub = "Dos redes, una radio."
    legal = "Meshtastic® + MeshCore · EU868"

    for text, font, y in ((title, title_font, 155), (sub, sub_font, 188), (legal, _font(11), 218)):
        bbox = draw.textbbox((0, 0), text, font=font)
        tw = bbox[2] - bbox[0]
        draw.text(((w - tw) // 2, y), text, fill=TEXT if y < 210 else (0x88, 0x8C, 0x94), font=font)
    return img


def png_to_xbm_lines(img: Image.Image, threshold: int = 128) -> tuple[int, int, list[int]]:
    gray = img.convert("L")
    w, h = gray.size
    bytes_per_row = (w + 7) // 8
    data: list[int] = []
    for y in range(h):
        for byte_ix in range(bytes_per_row):
            b = 0
            for bit in range(8):
                x = byte_ix * 8 + bit
                if x >= w:
                    continue
                if gray.getpixel((x, y)) < threshold:
                    b |= 1 << bit
            data.append(b)
    return w, h, data


def write_xbm_header(path: Path, width: int, height: int, data: list[int]) -> None:
    chunks = [f"0x{b:02x}" for b in data]
    wrapped = []
    line = "  "
    for i, c in enumerate(chunks):
        line += c + (", " if i < len(chunks) - 1 else "")
        if len(line) > 90:
            wrapped.append(line.rstrip(", "))
            line = "  "
    if line.strip():
        wrapped.append(line.rstrip(", "))
    content = (
        "#pragma once\n"
        "/* DCN OEM — XBM for OLED / USERPREFS_OEM */\n"
        f"#define DCN_OEM_IMAGE_WIDTH {width}\n"
        f"#define DCN_OEM_IMAGE_HEIGHT {height}\n"
        "#define DCN_OEM_IMAGE_DATA \\\n"
        + ", \\\n".join(wrapped)
        + "\n"
    )
    path.write_text(content, encoding="utf-8")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    render_splash().save(OUT / "logo_320x240.png", optimize=True)
    render_icon(512).save(OUT / "dcn_icon_512.png", optimize=True)
    render_icon(64).save(OUT / "dcn_icon_64.png", optimize=True)

    oem = render_icon(64).resize((64, 32), Image.Resampling.LANCZOS)
    ow, oh, odata = png_to_xbm_lines(oem)
    write_xbm_header(OUT / "dcn_oem.xbm.h", ow, oh, odata)
    print(f"Wrote assets to {OUT}")


if __name__ == "__main__":
    main()