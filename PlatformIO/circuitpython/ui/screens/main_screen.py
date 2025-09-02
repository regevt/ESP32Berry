import displayio
import terminalio
from adafruit_display_text import label


class MainScreen:
    """Home screen: top status bar + row of app icons.

    This is a visual approximation of the LVGL main screen.
    """

    def __init__(self, display):
        self.display = display
        self.group = displayio.Group()
        self.header = displayio.Group()
        self.body = displayio.Group(x=0, y=34)
        self.group.append(self.header)
        self.group.append(self.body)

        # Header/status bar
        self.title = label.Label(terminalio.FONT, text="ESP32Berry", color=0xFFFFFF, x=display.width//2 - 40, y=12)
        self.user = label.Label(terminalio.FONT, text="USER111", color=0xFFFFFF, x=display.width//2 - 20, y=24)
        self.noti = label.Label(terminalio.FONT, text="...", color=0xFFFFFF, x=display.width-40, y=12)
        self.header.append(self.title)
        self.header.append(self.user)
        self.header.append(self.noti)

        # Body app icon placeholders
        self.icon_group = displayio.Group(x=8, y=8)
        self.body.append(self.icon_group)
        self.icon_labels = []
        for i in range(5):
            lbl = label.Label(terminalio.FONT, text=f"[{i}]", color=0xFFFFFF, x=8 + i*60, y=20)
            self.icon_group.append(lbl)
            self.icon_labels.append(lbl)

    def set_notification(self, text: str):
        self.noti.text = text or ""

    def set_user(self, user: str):
        self.user.text = user or ""

    def get_group(self):
        return self.group
