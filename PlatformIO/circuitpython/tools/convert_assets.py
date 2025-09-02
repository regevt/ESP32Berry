#!/usr/bin/env python3
"""
Convert LVGL C image assets in src/UI/Assets into CircuitPython-compatible BMPs.

Supported formats:
- LV_COLOR_FORMAT_I8 (indexed 8bpp with RGBA palette): uses palette alpha as intensity
- LV_COLOR_FORMAT_RGB565: converts to 24-bit BMP

Output: PlatformIO/circuitpython/assets/<basename>.bmp
"""
import re
import os
import struct
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parents[2]
SRC_ASSETS = ROOT / "src" / "UI" / "Assets"
OUT_DIR = ROOT / "circuitpython" / "assets"

MAP_RE = re.compile(r"const\s+[^\[]+\s+(?P<name>\w+)_map\s*\[\s*\]\s*=\s*\{")
STRUCT_RE = re.compile(r"const\s+lv_image_dsc_t\s+(?P<name>\w+)\s*=\s*\{")
W_RE = re.compile(r"\.header\.w\s*=\s*(\d+)")
H_RE = re.compile(r"\.header\.h\s*=\s*(\d+)")
CF_RE = re.compile(r"\.header\.cf\s*=\s*([A-Za-z0-9_]+)")
DATA_SIZE_RE = re.compile(r"\.data_size\s*=\s*(\d+)")


def read_c_array_bytes(text: str) -> bytes:
    # Find the array block for <symbol_name>_map
    m = MAP_RE.search(text)
    if not m:
        raise ValueError("map array not found")
    start = m.end()
    # find matching closing brace of the array
    end = text.find("};", start)
    if end == -1:
        raise ValueError("array terminator not found")
    array_text = text[start:end]
    # remove comments and whitespace
    array_text = re.sub(r"/\*.*?\*/", "", array_text, flags=re.S)
    # keep only hex/dec numbers separated by commas
    nums = re.findall(r"0x[0-9A-Fa-f]+|\d+", array_text)
    b = bytes(int(n, 16) if n.startswith("0x") else int(n) for n in nums)
    return b


def parse_struct_meta(text: str):
    # parse width, height, color format, data_size
    w = W_RE.search(text)
    h = H_RE.search(text)
    cf = CF_RE.search(text)
    ds = DATA_SIZE_RE.search(text)
    if not (w and h and cf and ds):
        raise ValueError("failed to parse image metadata (w/h/cf/data_size)")
    return int(w.group(1)), int(h.group(1)), cf.group(1), int(ds.group(1))


def write_bmp24(path: Path, width: int, height: int, rgb_rows):
    """rgb_rows: list of rows, each row is bytes of length width*3 in top-to-bottom order"""
    # BMP stores bottom-up rows, each row padded to 4 bytes
    row_size = (width * 3 + 3) & ~3
    img_size = row_size * height
    # BITMAPFILEHEADER (14 bytes)
    bfType = b"BM"
    bfOffBits = 14 + 40  # headers only
    bfSize = bfOffBits + img_size
    file_header = struct.pack("<2sIHHI", bfType, bfSize, 0, 0, bfOffBits)
    # BITMAPINFOHEADER (40 bytes)
    biSize = 40
    biWidth = width
    biHeight = height
    biPlanes = 1
    biBitCount = 24
    biCompression = 0  # BI_RGB
    biSizeImage = img_size
    biXPelsPerMeter = 2835
    biYPelsPerMeter = 2835
    biClrUsed = 0
    biClrImportant = 0
    info_header = struct.pack(
        "<IIIHHIIIIII",
        biSize,
        biWidth,
        biHeight,
        biPlanes,
        biBitCount,
        biCompression,
        biSizeImage,
        biXPelsPerMeter,
        biYPelsPerMeter,
        biClrUsed,
        biClrImportant,
    )
    with open(path, "wb") as f:
        f.write(file_header)
        f.write(info_header)
        pad = b"\x00" * (row_size - width * 3)
        for row in reversed(rgb_rows):  # bottom-up
            f.write(row)
            if pad:
                f.write(pad)


def convert_i8_to_bmp(data: bytes, w: int, h: int, out_path: Path):
    # First 1024 bytes = 256 RGBA palette (R,G,B,A)
    if len(data) < 1024 + w * h:
        raise ValueError("I8 data too short")
    palette = data[:1024]
    indices = data[1024 : 1024 + w * h]
    rows = []
    for y in range(h):
        row_bgr = bytearray()
        base = y * w
        for x in range(w):
            idx = indices[base + x]
            r = palette[idx * 4 + 0]
            g = palette[idx * 4 + 1]
            b = palette[idx * 4 + 2]
            a = palette[idx * 4 + 3]
            # Composite over black using alpha
            r = (r * a) // 255
            g = (g * a) // 255
            b = (b * a) // 255
            row_bgr += bytes((b, g, r))
        rows.append(bytes(row_bgr))
    write_bmp24(out_path, w, h, rows)


def convert_rgb565_to_bmp(data: bytes, w: int, h: int, out_path: Path):
    expected = w * h * 2
    if len(data) < expected:
        raise ValueError("RGB565 data too short")
    rows = []
    off = 0
    for y in range(h):
        row_bgr = bytearray()
        for x in range(w):
            lo = data[off]
            hi = data[off + 1]
            off += 2
            val = hi << 8 | lo
            r = ((val >> 11) & 0x1F) * 255 // 31
            g = ((val >> 5) & 0x3F) * 255 // 63
            b = (val & 0x1F) * 255 // 31
            row_bgr += bytes((b, g, r))
        rows.append(bytes(row_bgr))
    write_bmp24(out_path, w, h, rows)


def convert_rgb565a8_to_bmp(data: bytes, w: int, h: int, out_path: Path):
    rgb_len = w * h * 2
    a_len = w * h
    if len(data) < rgb_len + a_len:
        raise ValueError("RGB565A8 data too short")
    rgb = data[:rgb_len]
    alpha = data[rgb_len: rgb_len + a_len]
    rows = []
    off = 0
    for y in range(h):
        row_bgr = bytearray()
        base_a = y * w
        for x in range(w):
            lo = rgb[off]
            hi = rgb[off + 1]
            off += 2
            val = hi << 8 | lo
            r = ((val >> 11) & 0x1F) * 255 // 31
            g = ((val >> 5) & 0x3F) * 255 // 63
            b = (val & 0x1F) * 255 // 31
            a = alpha[base_a + x]
            # composite over black: c' = c * a / 255
            r = (r * a) // 255
            g = (g * a) // 255
            b = (b * a) // 255
            row_bgr += bytes((b, g, r))
        rows.append(bytes(row_bgr))
    write_bmp24(out_path, w, h, rows)


def convert_file(c_path: Path) -> Optional[Path]:
    text = c_path.read_text()
    try:
        w, h, cf, data_size = parse_struct_meta(text)
    except Exception as e:
        print(f"[SKIP] {c_path.name}: {e}")
        return None
    try:
        data = read_c_array_bytes(text)
    except Exception as e:
        print(f"[SKIP] {c_path.name}: {e}")
        return None

    # Determine payload portion based on color format
    out_path = OUT_DIR / (c_path.stem + ".bmp")
    out_path.parent.mkdir(parents=True, exist_ok=True)
    if cf in ("LV_COLOR_FORMAT_I8", "LV_IMG_CF_INDEXED_8"):
        convert_i8_to_bmp(data, w, h, out_path)
    elif cf in ("LV_COLOR_FORMAT_RGB565", "LV_IMG_CF_TRUE_COLOR", "LV_IMG_CF_TRUE_COLOR_ALPHA"):
        # For TRUE_COLOR and TRUE_COLOR_ALPHA exports frequently use RGB565 in LVGL projects
        # Try as RGB565; if it fails, fall back to ARGB8888
        try:
            convert_rgb565_to_bmp(data, w, h, out_path)
        except Exception:
            # Try ARGB8888 (4 bytes/pixel)
            expected = w * h * 4
            if len(data) >= expected:
                rows = []
                off = 0
                for y in range(h):
                    row_bgr = bytearray()
                    for x in range(w):
                        r = data[off]
                        g = data[off + 1]
                        b = data[off + 2]
                        # a = data[off + 3]  # ignored
                        off += 4
                        row_bgr += bytes((b, g, r))
                    rows.append(bytes(row_bgr))
                write_bmp24(out_path, w, h, rows)
            else:
                raise
    elif cf in ("LV_COLOR_FORMAT_RGB565A8",):
        convert_rgb565a8_to_bmp(data, w, h, out_path)
    elif cf in ("LV_COLOR_FORMAT_ARGB8888",):
        expected = w * h * 4
        if len(data) < expected:
            raise ValueError("ARGB8888 data too short")
        rows = []
        off = 0
        for y in range(h):
            row_bgr = bytearray()
            for x in range(w):
                r = data[off]
                g = data[off + 1]
                b = data[off + 2]
                off += 4
                row_bgr += bytes((b, g, r))
            rows.append(bytes(row_bgr))
        write_bmp24(out_path, w, h, rows)
    else:
        print(f"[WARN] {c_path.name}: unsupported color format {cf}")
        return None

    print(f"[OK] {c_path.name} -> {out_path.relative_to(ROOT)}")
    return out_path


def main():
    if not SRC_ASSETS.exists():
        print(f"Source assets folder not found: {SRC_ASSETS}")
        return 1
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    converted = 0
    for cfile in sorted(SRC_ASSETS.glob("*.c")):
        if convert_file(cfile):
            converted += 1
    print(f"Converted {converted} file(s). Output: {OUT_DIR}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
