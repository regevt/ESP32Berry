# Pins and hardware config for T-Deck (CircuitPython)
TFT = {
    'width': 240,
    'height': 320,
    'invert': True,
    'pins': {
        'sclk': 40,
        'mosi': 41,
        'miso': 38,
        'dc': 11,
        'cs': 12,
        'rst': None,
        'bl': 42,
    },
}

TOUCH_GT911 = {
    'addr': 0x5D,
    'pins': {
        'sda': 18,
        'scl': 8,
        'int': 16,
    },
}

STORAGE = {
    'sd_pins': {
        'sclk': 40,
        'mosi': 41,
        'miso': 38,
        'cs': 39,
    },
}

AUDIO = {
    'pins': {
        'bck': 7,
        'ws': 5,
        'data': 6,
    }
}
