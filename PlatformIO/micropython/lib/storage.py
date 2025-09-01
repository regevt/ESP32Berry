# Storage helpers for MicroPython on ESP32
from .board_config import STORAGE

try:
    import machine
    import os
    import sdcard
except Exception:
    machine = os = sdcard = None  # type: ignore


def mount_sd(mount_point='/sd'):
    if machine is None or sdcard is None:
        raise RuntimeError('machine/sdcard not available')
    spi = machine.SPI(STORAGE['sd_spi_host'], baudrate=8_000_000,
                      sck=machine.Pin(STORAGE['sd_pins']['sclk']),
                      mosi=machine.Pin(STORAGE['sd_pins']['mosi']),
                      miso=machine.Pin(STORAGE['sd_pins']['miso']))
    sd = sdcard.SDCard(spi, machine.Pin(STORAGE['sd_pins']['cs']))
    vfs = os.VfsFat(sd)
    os.mount(vfs, mount_point)
    return mount_point
