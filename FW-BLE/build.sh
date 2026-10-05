#!/bin/bash
# Build the nRF52810 BLE firmware and merge it with the S112 SoftDevice into a
# single flashable hex. See BUILD.md for details.
#
# Usage:  ./build.sh [Debug|Release]      (default: Debug, same as released images)
# Output: FW-BLE-<YYMMDD>.hex in this folder
#
# Override the SES location with:  SES_DIR="/Applications/SEGGER/SEGGER Embedded Studio X.YY" ./build.sh
set -euo pipefail

CONFIG="${1:-Debug}"
HERE="$(cd "$(dirname "$0")" && pwd)"
PROJECT="$HERE/carss_board/s112/ses/FW-NIH-BLE.emProject"
APP_HEX="$HERE/carss_board/s112/ses/Output/$CONFIG/Exe/FW-NIH-BLE.hex"
SOFTDEVICE_HEX="$HERE/nRF5_SDK_17.1.0/components/softdevice/s112/hex/s112_nrf52_7.2.0_softdevice.hex"
OUT_HEX="$HERE/FW-BLE-$(date +%y%m%d).hex"

# Pick the newest installed SEGGER Embedded Studio unless SES_DIR is set.
if [ -z "${SES_DIR:-}" ]; then
    SES_DIR="$(ls -d /Applications/SEGGER/SEGGER\ Embedded\ Studio* 2>/dev/null | sort -V | tail -1 || true)"
fi
EMBUILD="$SES_DIR/bin/emBuild"
[ -x "$EMBUILD" ] || { echo "emBuild not found. Install SEGGER Embedded Studio or set SES_DIR." >&2; exit 1; }
command -v mergehex >/dev/null || { echo "mergehex not found. Install nRF Command Line Tools." >&2; exit 1; }

echo "== Building $CONFIG with $SES_DIR"
"$EMBUILD" -config "$CONFIG" -rebuild "$PROJECT"

echo "== Merging application + S112 SoftDevice"
mergehex -m "$SOFTDEVICE_HEX" "$APP_HEX" -o "$OUT_HEX"

echo "== Done: $OUT_HEX"
