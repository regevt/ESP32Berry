import displayio
import terminalio
from adafruit_display_text import label


class SecondScreen:
    def __init__(self, display):
        self.display = display
        self.group = displayio.Group()
        self.lbl = label.Label(terminalio.FONT, text="Second Screen", color=0xFFFFFF, x=8, y=12)
        self.group.append(self.lbl)

    def get_group(self):
        return self.group
