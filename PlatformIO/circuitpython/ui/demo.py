"""Small demo to preview the CircuitPython UI screens.
Upload and set code.py to import this demo to test visually.
"""
import time
from display import init_display
from ui.app_manager import ScreenManager
from ui.screens import MainScreen, SecondScreen, SettingsScreen


display = init_display()
sm = ScreenManager(display)

main = MainScreen(display)
sm.show_screen(main.get_group())
main.set_user("USER_NAME")
main.set_notification("Hello!")

# Cycle a couple of screens
for i in range(2):
    time.sleep(2)
    sm.show_screen(SecondScreen(display).get_group())
    time.sleep(2)
    settings = SettingsScreen(display)
    settings.set_brightness(200 - i * 40)
    settings.set_volume(10 + i * 5)
    settings.set_wifi(i % 2 == 0)
    sm.show_screen(settings.get_group())
    time.sleep(2)

sm.show_screen(main.get_group())
