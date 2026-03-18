#!/bin/bash
# 🎯 MT7902 WiFi & Bluetooth Automatic Fix for Linux
# This script compiles and loads the driver for the MediaTek MT7902 chip.

set -e

echo "🚀 Starting MT7902 Fix..."

# 1. Install prerequisites (for Ubuntu/Debian)
if [ -f /etc/debian_version ]; then
    echo "📦 Checking prerequisites..."
    sudo apt-get update
    sudo apt-get install -y build-essential linux-headers-$(uname -r) bc
fi

# 2. Compile WiFi Modules
echo "🛠️ Compiling WiFi modules..."
cd "$(dirname "$0")/latest"
make clean
make module_compile

# 3. Compile Bluetooth Modules
echo "🛠️ Compiling Bluetooth modules..."
BT_DIR="../linux-$(uname -r | cut -d'.' -f1,2)/drivers/bluetooth"
if [ -d "$BT_DIR" ]; then
    cd "$BT_DIR"
    make clean
    make
else
    echo "⚠️ Bluetooth source not found for this kernel version, skipping BT build."
fi

# 4. Prepare and Copy Modules
echo "📂 Installing modules..."
sudo mkdir -p /lib/modules/mt7902_custom/
cd "$(dirname "$0")/latest"
sudo cp *.ko /lib/modules/mt7902_custom/
sudo cp mt7921/*.ko /lib/modules/mt7902_custom/

if [ -d "$BT_DIR" ]; then
    cd "$BT_DIR"
    sudo cp btmtk.ko btusb.ko /lib/modules/mt7902_custom/
fi

# 5. Create/Update Setup Script
echo "📝 Configuring startup service..."
cat <<EOF | sudo tee /usr/local/bin/mt7902-setup.sh
#!/bin/bash
# Unload conflicting modules
rmmod btusb btmtk mt7921e mt7921_common mt792x_lib mt76_connac_lib mt76 2>/dev/null

# Load WiFi stack
modprobe cfg80211
modprobe mac80211

# Load custom MT7902 modules (WiFi)
insmod /lib/modules/mt7902_custom/mt76.ko
insmod /lib/modules/mt7902_custom/mt76-connac-lib.ko
insmod /lib/modules/mt7902_custom/mt792x-lib.ko
insmod /lib/modules/mt7902_custom/mt7921-common.ko
insmod /lib/modules/mt7902_custom/mt7921e.ko

# Load custom MT7902 modules (Bluetooth)
if [ -f /lib/modules/mt7902_custom/btmtk.ko ]; then
    insmod /lib/modules/mt7902_custom/btmtk.ko
    insmod /lib/modules/mt7902_custom/btusb.ko
    systemctl restart bluetooth
fi
EOF

sudo chmod +x /usr/local/bin/mt7902-setup.sh

# 6. Create systemd service
cat <<EOF | sudo tee /etc/systemd/system/mt7902.service
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

sudo systemctl daemon-reload
sudo systemctl enable mt7902.service
sudo systemctl restart mt7902.service

echo "✅ MT7902 is now active! Your WiFi should be working."
