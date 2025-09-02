import displayio
import terminalio
from adafruit_display_text import label


class WifiPanel:
    def __init__(self, width: int, height: int):
        self.width = width
        self.height = height
        self.group = displayio.Group()
        self.title = label.Label(terminalio.FONT, text="Wi-Fi", color=0xFFFFFF, x=6, y=12)
        self.group.append(self.title)
        self.list_group = displayio.Group(x=6, y=24)
        self.group.append(self.list_group)
        self.rows = []

    def set_networks(self, names):
        while len(self.list_group):
            self.list_group.pop()
        self.rows = []
        y = 0
        for ssid in names:
            row = label.Label(terminalio.FONT, text=ssid, color=0xFFFFFF, x=0, y=y+12)
            self.list_group.append(row)
            self.rows.append(row)
            y += 14

    def get_group(self):
        return self.group
