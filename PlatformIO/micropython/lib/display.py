# ST7789 and GT911 LVGL binding setup for T-Deck
# This assumes a MicroPython build with lvgl and esp32 drivers available.
# Pins from LGFX_T-Deck.h:
# SPI2: SCLK=40, MOSI=41, MISO=38, DC=11, CS=12, RST=-1 (unused), BL=42
# GT911: SDA=18, SCL=8, INT=16, addr=0x5D

import time
import sys
from .board_config import TFT

try:
    import lvgl as lv
except Exception as e:
    print('lvgl not available in host env (OK on dev PC):', e)
    lv = None  # type: ignore
try:
    import lvesp32  # noqa: F401
except Exception:
    pass
try:
    from machine import Pin, SPI, PWM
except Exception:
    # Allow linting on host
    Pin = SPI = PWM = object  # type: ignore

# Display init
class Display:
    def __init__(self):
        if lv is None:
            raise RuntimeError('lvgl not available - flash a MicroPython build with lvgl bindings')
        lv.init()
        # Backlight
        if PWM is object:
            raise RuntimeError('machine.PWM unavailable')
        self.bl = PWM(Pin(TFT['pins']['bl']))
        self.bl.freq(44100)
        self.set_backlight(0.9)

        # SPI for ST7789
        self.spi = SPI(TFT['spi_host'], baudrate=80_000_000, polarity=1, phase=1,
                       sck=Pin(TFT['pins']['sclk']), mosi=Pin(TFT['pins']['mosi']), miso=Pin(TFT['pins']['miso']))
        self.dc = Pin(TFT['pins']['dc'], Pin.OUT)
        self.cs = Pin(TFT['pins']['cs'], Pin.OUT)
        # Framebuffer driver: use lvesp32 driver helper
        try:
            from ili9xxx import st7789
        except Exception as e:
            print('ili9xxx driver missing:', e)
            raise

        self.disp = st7789(
            mosi=TFT['pins']['mosi'], miso=TFT['pins']['miso'], clk=TFT['pins']['sclk'],
            cs=TFT['pins']['cs'], dc=TFT['pins']['dc'],
            width=TFT['width'], height=TFT['height'], invert=TFT['invert'], rot=0, factor=16,
        )

        # Touch GT911 via indev if available
        try:
            from xpt2046 import XPT2046  # placeholder; GT911 driver varies by port
            self.touch = None
        except Exception:
            self.touch = None

    def set_backlight(self, level: float):
        level = max(0.0, min(1.0, level))
        duty = int(level * 65535)
        self.bl.duty_u16(duty)

    def width(self):
        return TFT['width']

    def height(self):
        return TFT['height']
