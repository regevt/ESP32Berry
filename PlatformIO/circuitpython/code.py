# code.py entry point for CircuitPython
# Runs the app and keeps it on screen
from app import main
import time

# Set to True to preview the new CircuitPython UI demo instead of file browser
USE_DEMO_UI = True

main(USE_DEMO_UI)

# Prevent returning to REPL console overlay
while True:
	time.sleep(1)
