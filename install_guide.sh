#!/bin/bash

# Exit the script if any command fails while navigating
set -e

# 1. Capture full kernel version (e.g., 6.14.2-generic)
FULL_KERNEL=$(uname -r)

# Extract just the Major.Minor version (e.g., 6.14 or 7.0)
KERNEL_VERSION=$(echo "$FULL_KERNEL" | cut -d. -f1,2)

# Define the dynamic path based on the kernel folder name
KERNEL_DIR="linux-$KERNEL_VERSION"

echo "Full Running Kernel: $FULL_KERNEL"
echo "Target Kernel Folder: $KERNEL_DIR"
echo "------------------------------------------------"

# Function to prompt the user for a Yes/No question
ask_user() {
    local prompt="$1"
    while true; do
        read -rp "$prompt [y/n]: " yn
        case $yn in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer yes (y) or no (n).";;
        esac
    done
}

# 2. Prompt for MT7902 Firmware
if ask_user "Do you want to install the firmware for MT7902 driver?"; then
    echo "Installing MT7902 firmware..."
    FW_DIR="./firmware/"
    
    if [ -f "./firmware/install_firmware.sh" ]; then
    	cd "$FW_DIR"
        chmod +x ./install_firmware.sh
        ./install_firmware.sh
        cd -> /dev/null
    else
        echo "Error: ./firmware/install_firmware.sh not found!"
    fi
else
    echo "Skipping MT7902 firmware installation."
fi

echo "------------------------------------------------"

# Check if the kernel folder actually exists before proceeding to drivers
if [ ! -d "./$KERNEL_DIR" ]; then
    echo "Error: Directory './$KERNEL_DIR' matching your kernel version ($KERNEL_VERSION) does not exist!"
    echo "Aborting script execution to avoid compilation failures."
    exit 1
fi

# 3. Prompt for Bluetooth Driver
if ask_user "Do you want to install the Bluetooth driver?"; then
    echo "Building Bluetooth driver..."
    BT_DIR="./$KERNEL_DIR/drivers/bluetooth/"
    
    if [ -d "$BT_DIR" ]; then
        cd "$BT_DIR"
        make
        cd - > /dev/null # Go back to the original root directory
    else
        echo "Error: Directory $BT_DIR does not exist!"
    fi
else
    echo "Skipping Bluetooth driver installation."
fi

echo "------------------------------------------------"

# 4. Prompt for Wi-Fi Driver
if ask_user "Do you want to install the Wi-Fi driver?"; then
    echo "Building Wi-Fi driver..."
    WIFI_DIR="./$KERNEL_DIR/drivers/net/wireless/mediatek/mt76"
    
    if [ -d "$WIFI_DIR" ]; then
        cd "$WIFI_DIR"
        make
        
        # Navigate to a sub-folder matching the specific kernel string if required
        if [ -d "$FULL_KERNEL" ]; then
            echo "Navigating to kernel-specific directory: $FULL_KERNEL"
            cd "$FULL_KERNEL"
        fi
    else
        echo "Error: Directory $WIFI_DIR does not exist!"
    fi
else
    echo "Skipping Wi-Fi driver installation."
fi

echo "------------------------------------------------"
echo "Script workflow completed."

