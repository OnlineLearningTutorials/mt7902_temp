#!/bin/bash

# Use firmware from latest/ — the older firmware in the parent directory
# causes MCU command timeouts and kernel panics.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
FW_DIR="$SCRIPT_DIR/latest"

cp -v "$FW_DIR/WIFI_MT7902_patch_mcu_1_1_hdr.bin.zst"  /lib/firmware/mediatek
cp -v "$FW_DIR/WIFI_RAM_CODE_MT7902_1.bin.zst"  /lib/firmware/mediatek
cp -v "$FW_DIR/BT_RAM_CODE_MT7902_1_1_hdr.bin.zst" /lib/firmware/mediatek

echo "Finished"

