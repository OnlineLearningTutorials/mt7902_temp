# mt7902 driver development
As mediatek wifi mt7902 wifi 6E works fine in windows 11, however linux does not implemented its driver as it is new and mediatek does not respond to making a driver for linux. We try to take mediatek driver code from kernel `linux-6.8.9` from folder `linux-6.8.9/drivers/net/wireless/mediatek` and trying to make a driver for 7902. 

## Compile and Build
Clone the repository to your local pc
    ```
    git clone https://github.com/OnlineLearningTutorials/mt7902_temp
    ```
If you don't want to clone past history than 
    ```
    git clone -depth=1 https://github.com/OnlineLearningTutorials/mt7902_temp
    ```

If you want to build the whole module than you just open the terminal in the main folder and execute `make` it will compile all modules of mt76 project. however if you want to only compile the mt7902 than go to folder `mt7902_temp/mt76/mt7902` and than execute the command `make`

## Kernel Module of Mediatek
    
    ./mt76/mt7603/mt7603e.ko
    ./mt76/mt7615/mt7615-common.ko
    ./mt76/mt7615/mt7615e.ko
    ./mt76/mt7615/mt7663s.ko
    ./mt76/mt7615/mt7663u.ko
    ./mt76/mt7615/mt7663-usb-sdio-common.ko
    ./mt76/mt76-connac-lib.ko
    ./mt76/mt76.ko
    ./mt76/mt76-sdio.ko
    ./mt76/mt76-usb.ko
    ./mt76/mt76x0/mt76x0-common.ko
    ./mt76/mt76x0/mt76x0e.ko
    ./mt76/mt76x0/mt76x0u.ko
    ./mt76/mt76x02-lib.ko
    ./mt76/mt76x02-usb.ko
    ./mt76/mt76x2/mt76x2-common.ko
    ./mt76/mt76x2/mt76x2e.ko
    ./mt76/mt76x2/mt76x2u.ko
    ./mt76/mt7902/mt7902-common.ko
    ./mt76/mt7902/mt7902e.ko
    ./mt76/mt7902/mt7902s.ko
    ./mt76/mt7902/mt7902u.ko
    ./mt76/mt7902_2/mt7902_2-common.ko
    ./mt76/mt7902_2/mt7902_2e.ko
    ./mt76/mt7902_2/mt7902_2u.ko
    ./mt76/mt7915/mt7915e.ko
    ./mt76/mt7921/mt7921-common.ko
    ./mt76/mt7921/mt7921e.ko
    ./mt76/mt7921/mt7921s.ko
    ./mt76/mt7921/mt7921u.ko
    ./mt76/mt7925/mt7925-common.ko
    ./mt76/mt7925/mt7925e.ko
    ./mt76/mt7925/mt7925u.ko
    ./mt76/mt792x-lib.ko
    ./mt76/mt792x-usb.ko
    ./mt76/mt7996/mt7996e.ko
    ./mt7601u/mt7601u.ko

In which the folder mt76 have shared modules as

    mt76-connac-lib.ko
    mt76.ko
    mt76-sdio.ko
    mt76-usb.ko
    mt76x02-lib.ko
    mt76x02-usb.ko
    mt792x-lib.ko
    mt792x-usb.ko
    
other modules are individual modules of particular chip

## Working with Kernel Modules in Linux
you can check currently loaded modules by `lsmod`, you can load a .ko kernel module by `insmod`. you can check if a module is already present in your kernel by `modinfo`. you can load already present module in kernel by `modprobe`

## Firmware for Wireless LAN_MTK_3.3.0.633_W11x64_(MTK7902)
`https://global-download.acer.com/GDFiles/Driver/Wireless%20LAN/Wireless%20LAN_MTK_3.3.0.633_W11x64_A.zip?acerid=638041027988808845&Step1=&Step2=&Step3=ASPIRE%20A314-23P&OS=ALL&LC=en&BC=ACER&SC=PA_6` windows driver for mtk7902 has following firmware

    WIFI_MT7922_patch_mcu_1_1_hdr.bin
    WIFI_MT7961_patch_mcu_1_2_hdr.bin
    WIFI_RAM_CODE_MT7922_1.bin
    WIFI_RAM_CODE_MT7961_1.bin
    
these files are already present in the `linux-firmware` package in Ubuntu 24.04 (Noble Numbat) as

    linux-firmware: /lib/firmware/mediatek/WIFI_MT7922_patch_mcu_1_1_hdr.bin.zst
    linux-firmware: /lib/firmware/mediatek/WIFI_MT7961_patch_mcu_1_2_hdr.bin.zst
    linux-firmware: /lib/firmware/mediatek/WIFI_RAM_CODE_MT7922_1.bin.zst
    linux-firmware: /lib/firmware/mediatek/WIFI_RAM_CODE_MT7961_1.bin.zst





