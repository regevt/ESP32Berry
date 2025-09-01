# Simple LVGL-based file browser in MicroPython
from lib.display import Display
from lib.fs import listdir
from lib.storage import mount_sd
from lib.net import wifi_connect
try:
    import secrets
except Exception:
    class secrets:
        WIFI_SSID = ''
        WIFI_PASSWORD = ''

try:
    import lvgl as lv
except Exception as e:
    raise

import os

class FileBrowser:
    def __init__(self, disp: Display, root='/'):
        self.disp = disp
        self.cur = root
        self.make_ui()
        self.refresh()

    def make_ui(self):
        self.screen = lv.obj()
        lv.scr_load(self.screen)

        self.list = lv.obj(self.screen)
        self.list.set_size(self.disp.width(), self.disp.height())
        self.list.set_scroll_dir(lv.DIR.VER)
        self.list.set_style_pad_all(0, 0)
        self.list.set_scrollbar_mode(lv.SCROLLBAR_MODE.ACTIVE)

    def clear_list(self):
        child = self.list.get_child(0)
        while child:
            nxt = child.get_next()
            child.delete()
            child = nxt

    def refresh(self):
        self.clear_list()
        entries = listdir(self.cur)
        y = 10
        for name in entries:
            if str(name).startswith('.'):
                continue
            # Best-effort directory check
            is_dir = False
            try:
                import stat
                st = os.stat(self.cur + '/' + name)
                is_dir = (st[0] & 0x4000) != 0
            except Exception:
                pass
            if is_dir:
                icon = lv.label(self.list)
                icon.set_text('📁')
                icon.set_pos(10, y)
            lbl = lv.label(self.list)
            lbl.set_text(str(name))
            lbl.set_pos(38, y)
            y += 22


def main():
    d = Display()
    if secrets.WIFI_SSID:
        connected, cfg = wifi_connect(secrets.WIFI_SSID, secrets.WIFI_PASSWORD)
        print('WiFi:', 'connected' if connected else 'not connected', cfg)
    root = '/'
    try:
        root = mount_sd()
        print('SD mounted at', root)
    except Exception as e:
        print('SD mount failed:', e)
    fb = FileBrowser(d, root=root)

if __name__ == '__main__':
    main()
