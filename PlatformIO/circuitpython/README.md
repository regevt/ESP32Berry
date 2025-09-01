# CircuitPython port for ESP32Berry T-Deck

This folder contains a CircuitPython application that mirrors the MicroPython scaffold:
- displayio + framebufferio with ST7789
- GT911 touch via I2C + adafruit_displayio_shape fallback pointer
- SD card via storage + sdcardio
- Simple file browser UI and WAV playback via audiocore + audiobusio.I2SOut

Copy all files to CIRCUITPY. The board must expose the USB drive.
