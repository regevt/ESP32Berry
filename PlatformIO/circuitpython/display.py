import board, busio, digitalio
import displayio
from adafruit_st7789 import ST7789
from .board_config import TFT


def init_display():
    displayio.release_displays()
    spi = busio.SPI(board.IO40, MOSI=board.IO41, MISO=board.IO38)
    tft_dc = digitalio.DigitalInOut(board.IO11)
    tft_dc.direction = digitalio.Direction.OUTPUT
    tft_cs = digitalio.DigitalInOut(board.IO12)
    tft_cs.direction = digitalio.Direction.OUTPUT
    display_bus = displayio.FourWire(spi, command=tft_dc, chip_select=tft_cs, baudrate=48_000_000, polarity=1, phase=1)

    display = ST7789(display_bus, width=TFT['width'], height=TFT['height'], rotation=0, rowstart=0, colstart=0)
    if TFT['invert']:
        display.invert(True)

    # Backlight
    bl = digitalio.DigitalInOut(board.IO42)
    bl.direction = digitalio.Direction.OUTPUT
    bl.value = True

    return display
