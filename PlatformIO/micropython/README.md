# MicroPython port for ESP32Berry T-Deck

This folder contains a minimal MicroPython application scaffolding that mirrors key features of the C++ project using LVGL and hardware drivers, while allowing selective reuse of C++ via wrapped modules later.

Contents
- boot.py: basic boot setup
- main.py: LVGL init, display, touch, and a simple file browser UI
- lib/: shared MicroPython helpers
- README.md: notes and next steps

Prereqs
- Flash a MicroPython firmware that includes LVGL and ili9xxx display driver (e.g., lv_micropython for ESP32-S3).

Build lv_micropython (ESP32-S3 example)
1) Clone https://github.com/lvgl/lv_micropython
2) make -C mpy-cross
3) cd ports/esp32
4) export IDF_TARGET=esp32s3
5) make submodules
6) make BOARD=GENERIC_S3 LV_CFLAGS="-DLV_COLOR_16_SWAP=1"

Flash firmware (adjust port)
esptool.py --chip esp32s3 --port /dev/tty.usbmodem* erase_flash
esptool.py --chip esp32s3 --port /dev/tty.usbmodem* --baud 460800 write_flash -z 0x0 build-GENERIC_S3/firmware.bin

Upload project files
mpremote cp -r PlatformIO/micropython/* :

Run
The board runs boot.py then main.py automatically.

Native C/C++ wrappers
- See native/esp32berry.c; integrate via lv_micropython user_modules to call into optimized C++ if needed.
