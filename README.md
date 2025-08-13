# mt7902 driver development (in progress)
We are trying to develop the driver for the Mediatek mt7902 wifi 6E chip

## Firmwares used
Firmwares are stored in `mt7902_firmware` folder.

## Compile and Build
Clone the repository to your local pc
  ```
  git clone https://github.com/OnlineLearningTutorials/mt7902_temp
  ```
If you don't want to clone past history than 
  ```
  git clone --depth 1 https://github.com/OnlineLearningTutorials/mt7902_temp
  ```

If you want to build the whole module than you just open the terminal in the main folder and execute `make` it will compile all modules of mt76 project. however if you want to only compile the mt7902 than go to folder `mt7902_temp/mt76/mt7902` and than execute the command `make`


## Working with Kernel Modules in Linux
you can check currently loaded modules by `lsmod`. 
you can load a .ko kernel module by `insmod`. 
you can check if a module is already present in your kernel by `modinfo`. 
you can load already present module in kernel by `modprobe`. 
Insert the module in your kernel by 

    sudo modprobe mt76-connac-lib
    sudo modprobe mt76
    sudo modprobe mt76-sdio
    sudo modprobe mt76-usb
    sudo modprobe mt76x02-lib
    sudo modprobe mt76x02-usb
    sudo modprobe mt792x-lib
    sudo modprobe mt792x-usb



## Bluetooth
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