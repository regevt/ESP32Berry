# T-Deck board configuration for MicroPython port
# Sourced from src/Configurations/LGFX_T-Deck.h

TFT = {
    'width': 240,
    'height': 320,
    'invert': True,
    'spi_host': 2,  # SPI2
    'pins': {
        'sclk': 40,
        'mosi': 41,
        'miso': 38,
        'dc': 11,
        'cs': 12,
        'rst': -1,
        'bl': 42,
    },
}

TOUCH_GT911 = {
    'addr': 0x5D,
    'i2c_port': 0,
    'pins': {
        'int': 16,
        'sda': 18,
        'scl': 8,
    },
    'freq': 400_000,
}

# Storage (SPI SD)
STORAGE = {
    'sd_spi_host': 2,
    'sd_pins': {
        'sclk': 40,
        'mosi': 41,
        'miso': 38,
        'cs': 39,
    },
}
