# ST7789 and GT911 LVGL binding setup for T-Deck
# This assumes a MicroPython build with lvgl and esp32 drivers available.
# Pins from LGFX_T-Deck.h:
# SPI2: SCLK=40, MOSI=41, MISO=38, DC=11, CS=12, RST=-1 (unused), BL=42
# GT911: SDA=18, SCL=8, INT=16, addr=0x5D

import time
import sys

try:
    import lvgl as lv
    import lvesp32
    from machine import Pin, SPI, I2C, PWM
except ImportError as e:
    print('Missing MicroPython lvgl stack:', e)
    raise

# Display init
class Display:
    def __init__(self):
        lv.init()
        # Backlight
        self.bl = PWM(Pin(42))
        self.bl.freq(44100)
        self.set_backlight(0.9)

        # SPI for ST7789
        self.spi = SPI(2, baudrate=80_000_000, polarity=1, phase=1, sck=Pin(40), mosi=Pin(41), miso=Pin(38))
        self.dc = Pin(11, Pin.OUT)
        self.cs = Pin(12, Pin.OUT)
        # Framebuffer driver: use lvesp32 driver helper
        try:
            from ili9xxx import st7789
        except Exception as e:
            print('ili9xxx driver missing:', e)
            raise

        self.disp = st7789(
            mosi=41, miso=38, clk=40, cs=12, dc=11,
            width=240, height=320, invert=True, rot=0, factor=16,
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
        return 240

    def height(self):
        return 320
