#!/usr/bin/env python3
import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
secrets_h = ROOT / 'src' / 'Configurations' / 'secrets.h'
mpy_secrets = ROOT / 'PlatformIO' / 'micropython' / 'secrets.py'

vals = {}
if secrets_h.exists():
    txt = secrets_h.read_text()
    for k, v in re.findall(r'#define\s+(\w+)\s+"([^"]*)"', txt):
        vals[k] = v

ssid = vals.get('WIFI_SSID', os.getenv('PIO_WIFI_SSID', ''))
password = vals.get('WIFI_PASSWORD', os.getenv('PIO_WIFI_PASSWORD', ''))

mpy_secrets.write_text(f'WIFI_SSID = "{ssid} "\nWIFI_PASSWORD = "{password}"\n')
print('Wrote', mpy_secrets)
