#!/usr/bin/env bash
set -euo pipefail

# Copy CircuitPython app to mounted CIRCUITPY volume on macOS.
VOL="/Volumes/CIRCUITPY"
if [ ! -d "$VOL" ]; then
  echo "CIRCUITPY not mounted at $VOL" >&2
  exit 1
fi
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
APP="$ROOT"
set -x
rsync -av --delete "$APP/code.py" "$VOL/"
rsync -av --delete "$APP/app.py" "$VOL/"
rsync -av --delete "$APP/display.py" "$VOL/"
rsync -av --delete "$APP/storage_cp.py" "$VOL/"
rsync -av --delete "$APP/wav_player_cp.py" "$VOL/"
rsync -av --delete "$APP/board_config.py" "$VOL/"
rsync -av --delete "$APP/secrets.py" "$VOL/"
set +x
sync
