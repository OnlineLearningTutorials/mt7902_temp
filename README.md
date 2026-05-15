# 🎯 MT7902 Driver as Sub-Part of MT7921 Driver (✅ Working)
Mediatek mt7902 wifi 6E chip Bluetooth and WiFi Support

> [!IMPORTANT]
> Linux 7.1 Kernel Officially Supports the MT7902 WIFI 6E chip


## 📁 Cloning the repository
Clone the repository to your local pc
  ```
  git clone https://github.com/OnlineLearningTutorials/mt7902_temp
  ```
If you don't want to clone past history than 
  ```
  git clone --depth 1 https://github.com/OnlineLearningTutorials/mt7902_temp
  ```

## Installation


### 🚀 Installation Guide Script (Recommended for Ubuntu Based System)
Make the script executable if not already
```
chmod +x ./install_guide.sh
```
Now run this script
```
./install_guide.sh
```
It will automatically prompt for password when necessary
* This script will ask you to install firmware, bluetooth and wifi driver one by one
* Tested on Ubuntu with linux 7.0 kernel

### 🚀 Easy Automatic Fix (Recommended for Arch Based System)
If you want to quickly fix your WiFi and Bluetooth on any modern kernel, follow these steps:

Make the script executable if not already
```
chmod +x ./fix_my_wifi.sh
```

**Run the automatic fix script** with sudo:
```bash
sudo bash fix_my_wifi.sh
```

#### 📖 What this script does:
* **Checks for dependencies:** Ensures you have `gcc`/`clang`, `make`, `bc`, and your current `kernel-headers` installed. If not - installs with ur preffered package manager
* **Compiles Drivers:** Automatically builds both WiFi and Bluetooth drivers for your exact kernel version.
* **Persistent Fix:** Installs a system service that ensures your WiFi stays active even after you restart your computer.
* **Safety:** Installs modules into a custom directory (`/lib/modules/mt7902_custom`) to avoid messing with your default system files.

> [!NOTE]
> You will need an internet connection (via Ethernet or USB tethering from your phone) the first time you run this to download the necessary build tools. (Such as compilator, linux-headers, etc)


## ✅ Tested On (Verified Working)
This fix has been verified and is confirmed working on:

* **Brand:** ASUS
* **Model:** Vivobook Go (E1404FA), Vivobook 14 (X1404ZA)
* **Chipset:** MediaTek MT7902 (WiFi 6E)
* **Kernel Version:** 6.19.0 (Linux), 6.19.11, 7.0.7
* **OSes:** Arch, Ubuntu
* **Package Manager:** pacman, apt

## Available for:
* **OS**: Any os that support one of PM`s
* **Preffered package managers**: apt, pacman, dnf, zypper, nix-shell
* **Kernel versions**: 6.14-7.0.7


## 🔧 Firmwares used
Firmwares are stored in `firmware` folder.


## 📱 Bluetooth ✅ (Working)
> [!WARNING]
> If bluetooth driver conflict with `gen4-mt7902` than please remove the bluetooth firmware so that it not interfere with this driver
> ``` sudo rm /lib/firmware/mediatek/mt7902/BT_RAM_CODE_MT7902_1_1_hdr.bin.zst ```
> This project uses the firmware
> ``` /lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin.zst ```

To enable bluetooth go to the directory of your current kernel version. ``
Like if you have kernel linux-6.16 than go to the directory `./linux-6.16/drivers/bluetooth` .
Open terminal in this directory and compile with command `make`.
Two kernel modules are compiled `btusb.ko` and `btmtk.ko`.
To enable bluetooth in your device remove the btusb and btmtk in your system and install these two files, use commands
```
sudo rmmod btusb 
sudo rmmod btmtk

sudo insmod btmtk.ko
sudo insmod btusb.ko

```
Now check your bluetooth is working now.

## 💻 WiFi ✅ (Working)
> [!IMPORTANT]
> A working repo with some limitation is [here](https://github.com/hmtheboy154/gen4-mt7902)

WiFi driver for the mt7902, recently released by mediatek is inside the `latest` folder. 

If you are using Ubuntu than just go to the `latest` folder and run the following command in the termianl. 
```
make
```

It will compile all modules, compress it and install it (replace original kernel module with the modified module). If you are some other distro or not want all steps and only wants to compile the code, than run in the termianl 
```
make module_compile
```
To compress the module you compiled, than run in terminal
```
make module_compress
```
To install the compressed module to the system's kernel module, run in terminal
```
make module_install
```
