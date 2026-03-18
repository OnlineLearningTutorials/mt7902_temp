# 🎯 mt7902 driver development (✅ Working)
We are trying to develop the driver for the Mediatek mt7902 wifi 6E chip

## 🚀 Easy Automatic Fix (Recommended)
If you want to quickly fix your WiFi and Bluetooth on any modern kernel, follow these steps:

1. **Open your terminal** and clone the repository:
   ```bash
   git clone --depth 1 https://github.com/OnlineLearningTutorials/mt7902_temp
   cd mt7902_temp
   ```

2. **Run the automatic fix script** with sudo:
   ```bash
   sudo bash fix_my_wifi.sh
   ```

### 📖 What this script does:
* **Checks for dependencies:** Ensures you have `gcc`, `make`, and your current `kernel-headers` installed.
* **Compiles Drivers:** Automatically builds both WiFi and Bluetooth drivers for your exact kernel version.
* **Persistent Fix:** Installs a system service that ensures your WiFi stays active even after you restart your computer.
* **Safety:** Installs modules into a custom directory (`/lib/modules/mt7902_custom`) to avoid messing with your default system files.

> [!NOTE]
> You will need an internet connection (via Ethernet or USB tethering from your phone) the first time you run this to download the necessary build tools.

## 🔧 Firmwares used
Firmwares are stored in `mt7902_firmware` folder.
Recently released firmware are in the `mt7902_firmware/latest` folder.

## 📁 Cloning the repository
Clone the repository to your local pc
  ```
  git clone https://github.com/OnlineLearningTutorials/mt7902_temp
  ```
If you don't want to clone past history than 
  ```
  git clone --depth 1 https://github.com/OnlineLearningTutorials/mt7902_temp
  ```


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
