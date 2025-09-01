# MicroPython port for ESP32Berry T-Deck

This folder contains a minimal MicroPython application scaffolding that mirrors key features of the C++ project using LVGL and hardware drivers, while allowing selective reuse of C++ via wrapped modules later.

Contents
- boot.py: basic boot setup
- main.py: LVGL init, display, touch, and a simple file browser UI
- lib/: shared MicroPython helpers
- README.md: notes and next steps

Upload
Use mpremote or rshell to copy the files to the board running a MicroPython firmware with LVGL bindings.
