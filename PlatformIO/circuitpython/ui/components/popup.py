import displayio
import terminalio
from adafruit_display_text import label


class Popup:
    def __init__(self, width: int, height: int):
        self.group = displayio.Group()
        # Simple dark rect background
        # In DisplayIO, a real rectangle requires a Bitmap+Palette + TileGrid
        self.label_title = label.Label(terminalio.FONT, text="", color=0xFFFFFF, x=6, y=14)
        self.label_msg = label.Label(terminalio.FONT, text="", color=0xFFFFFF, x=6, y=32)
        self.ok = label.Label(terminalio.FONT, text="[OK]", color=0xFFFF00, x=width-40, y=height-8)
        self.group.append(self.label_title)
        self.group.append(self.label_msg)
        self.group.append(self.ok)

    def open(self, title: str, msg: str):
        self.label_title.text = title or ""
        self.label_msg.text = msg or ""

    def get_group(self):
        return self.group
