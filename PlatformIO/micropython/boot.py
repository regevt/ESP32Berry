# Minimal boot script for MicroPython on ESP32-S3 T-Deck
import sys
import machine

# Optionally tweak CPU freq or power saving here
try:
    machine.freq(240_000_000)
except Exception as e:
    print('freq set failed:', e)

print('boot.py done')
