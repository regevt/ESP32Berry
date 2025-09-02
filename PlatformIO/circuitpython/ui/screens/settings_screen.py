import displayio
import terminalio
from adafruit_display_text import label


class SettingsScreen:
    """Settings UI approximation: brightness and volume sliders and WiFi toggle placeholder.
    Note: CircuitPython sliders require a custom widget; we render labels and capture taps in app.
    """

    def __init__(self, display):
        self.display = display
        self.group = displayio.Group()
        # Title
        self.title = label.Label(terminalio.FONT, text="Settings", color=0xFFFFFF, x=8, y=12)
        self.group.append(self.title)
        # Brightness
        self.bright_lbl = label.Label(terminalio.FONT, text="Brightness:", color=0xCCCCCC, x=8, y=32)
        self.group.append(self.bright_lbl)
        self.bright_val = label.Label(terminalio.FONT, text="255", color=0xFFFFFF, x=120, y=32)
        self.group.append(self.bright_val)
        # Volume
        self.vol_lbl = label.Label(terminalio.FONT, text="Volume:", color=0xCCCCCC, x=8, y=50)
        self.group.append(self.vol_lbl)
        self.vol_val = label.Label(terminalio.FONT, text="21", color=0xFFFFFF, x=120, y=50)
        self.group.append(self.vol_val)
        # WiFi
        self.wifi_lbl = label.Label(terminalio.FONT, text="WiFi:", color=0xCCCCCC, x=8, y=70)
        self.group.append(self.wifi_lbl)
        self.wifi_state = label.Label(terminalio.FONT, text="OFF", color=0xFF5555, x=120, y=70)
        self.group.append(self.wifi_state)

    def set_brightness(self, val: int):
        self.bright_val.text = str(val)

    def set_volume(self, val: int):
        self.vol_val.text = str(val)

    def set_wifi(self, on: bool):
        self.wifi_state.text = "ON" if on else "OFF"
        self.wifi_state.color = 0x55FF55 if on else 0xFF5555

    def get_group(self):
        return self.group
