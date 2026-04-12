#!/bin/bash
# 🎯 MT7902 WiFi & Bluetooth Automatic Fix for Linux
# =================================================
# This script automates the process of fixing the MediaTek MT7902 driver
# issues on modern Linux kernels (like 6.19+).
#
# 📖 USER GUIDE:
# 1. Open your terminal in the directory where this script is located.
# 2. Make the script executable:
#    chmod +x fix_my_wifi.sh
# 3. Run the script with sudo:
#    sudo ./fix_my_wifi.sh
#
# 🛠️ WHAT THIS SCRIPT DOES:
# - Installs build dependencies (gcc, make, kernel headers).
# - Compiles the WiFi and Bluetooth drivers for your specific kernel.
# - Installs the drivers into a safe system directory (/lib/modules/mt7902_custom).
# - Sets up a systemd service to ensure WiFi works after every reboot.
#
# ⚠️ PREREQUISITES:
# - An active internet connection (via Ethernet or USB tethering) is required 
#   the first time you run this to download kernel headers.
# =================================================

set -e

# Variables declaration
SCRIPT_DIR=$(pwd)
KERNEL_VER=$(uname -r)
IS_CACHYOS=false
IS_ARCH=false
BT_DIR="$SCRIPT_DIR/linux-$KERNEL_VER/drivers/bluetooth"

if [[ "$KERNEL_VER" == *"cachyos"* ]]; then
    IS_CACHYOS=true
fi

if [[ "$KERNEL_VER" == *"arch"* ]]; then
    IS_ARCH=true
fi

# Usage Check: Ensure script is run with sudo
if [[ $EUID -ne 0 ]]; then
    echo "❌ This script must be run as root (use sudo)."
    echo "Usage: sudo ./fix_my_wifi.sh"
    exit 1
fi

echo "🚀 Starting MT7902 Fix..."

# 1. Install prerequisites
# For Ubuntu/Debian
if [ -f /etc/debian_version ]; then
    echo "📦 Checking prerequisites..."
    apt-get update
    apt-get install -y build-essential linux-headers-$KERNEL_VER bc
fi

# For CachyOS and Arch
if $IS_ARCH || $IS_CACHYOS; then
  sudo pacman -S --noconfirm clang llvm lld linux-headers base-devel
fi

# 2. Compile WiFi Modules
echo "🛠️ Compiling WiFi modules..."
cd "$SCRIPT_DIR/latest"
make clean || echo "No clean target for WiFi, continuing..."
if $IS_CACHYOS; then
    make CC=clang LD=ld.lld module_compile
else
    make module_compile
fi

# 3. Compile Bluetooth Modules
echo "🛠️ Compiling Bluetooth modules..."
if [ -d "$BT_DIR" ]; then
    cd "$BT_DIR"
    make clean || echo "No clean target for BT, continuing..."
    if $IS_CACHYOS; then
        make CC=clang LD=ld.lld
    else
        make
    fi
else
    echo "⚠️ Bluetooth source not found for this kernel version, skipping BT build."
fi

# 4. Prepare and Copy Modules
echo "📂 Installing modules..."
MODULE_DIR="/lib/modules/mt7902_custom"
mkdir -p "$MODULE_DIR"
cd  "$SCRIPT_DIR/latest"
cp *.ko "$MODULE_DIR/"
cp mt7921/*.ko "$MODULE_DIR/"

if [ -d "$BT_DIR" ]; then
    cd "$BT_DIR"
    cp btmtk.ko btusb.ko /lib/modules/mt7902_custom/
fi

# 5. Create/Update Setup Script
echo "📝 Configuring startup service..."
cat <<EOF | tee /usr/local/bin/mt7902-setup.sh
#!/bin/bash
# Unload conflicting modules
rmmod btusb btmtk mt7921e mt7921_common mt792x_lib mt76_connac_lib mt76 2>/dev/null || true

# Load WiFi stack
modprobe cfg80211
modprobe mac80211

# Load custom MT7902 modules (WiFi)
insmod /lib/modules/mt7902_custom/mt76.ko
insmod /lib/modules/mt7902_custom/mt76-connac-lib.ko
insmod /lib/modules/mt7902_custom/mt792x-lib.ko
insmod /lib/modules/mt7902_custom/mt7921-common.ko
insmod /lib/modules/mt7902_custom/mt7921e.ko

# Load Blutetooth stack if modules exist
if [ -f /lib/modules/mt7902_custom/btmtk.ko ]; then
    modprobe bluetooth
    modprobe btrtl
    modprobe btintel
    modprobe btbcm

    insmod /lib/modules/mt7902_custom/btmtk.ko
    insmod /lib/modules/mt7902_custom/btusb.ko

    systemctl restart bluetooth || true
fi
EOF

chmod +x /usr/local/bin/mt7902-setup.sh

# 6. Create systemd service
cat <<EOF | tee /etc/systemd/system/mt7902.service
[Unit]
Description=Load custom MT7902 Bluetooth and Wi-Fi drivers
After=network.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/mt7902-setup.sh
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable mt7902.service
systemctl restart mt7902.service

echo "✅ MT7902 is now active! Your WiFi and Bluetooth should be working."