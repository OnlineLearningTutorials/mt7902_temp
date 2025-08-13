#!/bin/bash

cp -v WIFI_MT7902_patch_mcu_1_1_hdr.bin.zst  /lib/firmware/mediatek
cp -v WIFI_RAM_CODE_MT7902_1.bin.zst  /lib/firmware/mediatek
cp -v BT_RAM_CODE_MT7902_1_1_hdr.bin.zst /lib/firmware/mediatek

echo "Finished"

