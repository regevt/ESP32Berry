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

# Optional: set CLEAN=1 to enable deletions on the device for removed files
# Change detection strategy (default: size). Options: size | mtime | checksum
#   size:      fastest; only size compared (good for CIRCUITPY FAT timestamp quirks)
#   mtime:     compare mod-time with a 2s window (FAT resolution)
#   checksum:  compute file checksums (slow but accurate if sizes match)
DETECT_MODE=${DETECT:-size}
RSYNC_FLAGS=( -a )
if [ "${VERBOSE:-1}" = "1" ]; then
  RSYNC_FLAGS+=( -v )
fi
case "$DETECT_MODE" in
  size) RSYNC_FLAGS+=( --size-only ) ;;
  mtime) RSYNC_FLAGS+=( --modify-window=2 ) ;;
  checksum) RSYNC_FLAGS+=( -c ) ;;
  *) RSYNC_FLAGS+=( --size-only ) ;;
esac
if [ "${CLEAN:-0}" = "1" ]; then
  RSYNC_FLAGS+=( --delete )
fi

set -x
# Use a single rsync with include/exclude so only our app files/dirs are considered.
# Unchanged files are skipped by rsync (mtime/size), so only modified files transfer.
rsync "${RSYNC_FLAGS[@]}" \
  --include='code.py' \
  --include='app.py' \
  --include='display.py' \
  --include='storage_cp.py' \
  --include='wav_player_cp.py' \
  --include='board_config.py' \
  --include='secrets.py' \
  --include='ui/***' \
  --include='assets/***' \
  --exclude='*' \
  "$APP/" "$VOL/"
set +x
sync
