# WiFi helper for MicroPython ESP32
import time
import network


def wifi_connect(ssid, password, timeout=10):
    sta = network.WLAN(network.STA_IF)
    if not sta.active():
        sta.active(True)
    if not sta.isconnected():
        sta.connect(ssid, password)
        t0 = time.ticks_ms()
        while not sta.isconnected() and time.ticks_diff(time.ticks_ms(), t0) < timeout * 1000:
            time.sleep_ms(200)
    return sta.isconnected(), sta.ifconfig() if sta.isconnected() else None
