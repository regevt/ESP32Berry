import displayio
import terminalio
from adafruit_display_text import label
from display import init_display
from storage_cp import mount_sd
from wav_player_cp import WavPlayer
from ui.app_manager import ScreenManager
from ui.screens import MainScreen, SecondScreen, SettingsScreen

import os

class FileBrowser:
    def __init__(self):
        self.display = init_display()
        self.root = "/"
        try:
            self.root = mount_sd()
        except Exception as e:
            print("SD mount failed:", e)
        self.cur = self.root
        self.player = WavPlayer()
    self.make_ui()
    self.refresh()

    def make_ui(self):
        self.group = displayio.Group()
        try:
            # CircuitPython 8 and earlier
            self.display.show(self.group)
        except AttributeError:
            # CircuitPython 9+: use root_group property
            self.display.root_group = self.group
        # Header
        self.header = displayio.Group()
        self.group.append(self.header)
        self.path_lbl = label.Label(terminalio.FONT, text=self.cur, color=0xFFFFFF, x=4, y=12)
        self.header.append(self.path_lbl)
        self.up_lbl = label.Label(terminalio.FONT, text="[Up]", color=0xFFFF00, x=self.display.width-40, y=12)
        self.header.append(self.up_lbl)
        # Body
        self.body = displayio.Group(x=0, y=20)
        self.group.append(self.body)

    def clear_body(self):
        while len(self.body):
            self.body.pop()

    def refresh(self):
        self.clear_body()
        self.path_lbl.text = self.cur
        y = 0
        try:
            entries = os.listdir(self.cur)
        except Exception:
            entries = []
        for name in sorted(entries):
            if name.startswith('.'):
                continue
            full = self.cur.rstrip('/') + '/' + name
            is_dir = False
            try:
                st = os.stat(full)
                is_dir = (st[0] & 0x4000) != 0
            except Exception:
                pass
            row = label.Label(terminalio.FONT, text=("[D] " if is_dir else "    ") + name, color=0xFFFFFF, x=4, y=y+12)
            self.body.append(row)
            y += 14

    # For simplicity, no touch handling here; use a keyboard or rotary in a later step


def main(use_demo_ui: bool = False):
    if use_demo_ui:
        disp = init_display()
        sm = ScreenManager(disp)
        main_screen = MainScreen(disp)
        sm.show_screen(main_screen.get_group())
        return
    fb = FileBrowser()

