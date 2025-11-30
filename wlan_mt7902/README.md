> [!CAUTION]
> This driver is still in Work-In-Progress state ! Using it requires extra work which will be explained below. It is not suitable for everyday use yet !

# gen4-mt7902

This is a driver for the Mediatek MT7902 PCIe card based on the `gen4-mt79xx` driver Mediatek gave for Android OEMs. This driver is extracted from [Xiaomi's rodin BSP](https://github.com/MiCode/MTK_kernel_modules/tree/bsp-rodin-v-oss/connectivity/wlan/core/gen4-mt79xx) and is being modified to focusing on supporting the MT7902 card only. The main goal of the project is to provide an usable driver for this particular M.2 PCIe card which is not available on the [mt76 driver](https://wireless.docs.kernel.org/en/latest/en/users/drivers/mediatek.html) yet.

## Status

The driver is buildable and loadable. It can be able to connect to 2.4Ghz wifi so far. However, upon testing, I've noticed these issues:

- Lack of power management making sleep broke & on next restart the driver is broken when being loaded. (you have to force shut down using power button to fix this).
- Can't switch to 5Ghz if you are on a SSID with both 2.4/5.
- Can't be able to connect to WPA3 networks.
- Can't create wifi hotspot to act as a repeater.
- Chunky compiled size with almost ~100mb, might be due to the debug code it has.

> [!WARNING]
> Because of the first issue, you have to use `sudo rmmod mt7902` whenever you wanna sleep or shut down the device !

There are some features that are untested such as Bluetooth (which is not covered by this driver) and WIFI 6/6E.

## Installation guide

> [!IMPORTANT]
> Before building & installing this driver, remember to install essential packages to build a kernel driver like linux kernel's headers & toolchain. I will not cover it here.

- Get the source by using `git`
```
git clone https://github.com/hmtheboy154/gen4-mt7902
cd gen4-mt7902
```

- To only build the driver, use this command
```
make -j$(nproc)
```

- To build the driver & install it, use this command
```
sudo make install -j$(nproc)
```

- To install the firmware required for the driver, use this command
```
sudo make install_fw
```

Once you got the driver & firmware installed, reboot to see changes.

## Tested hardware

Currently the driver is being tested on some of these models:
- WMDM-257AX (tested without antenna connected)

## FAQs

### What's the minimum kernel version that the driver is going to aim ?
I'm thinking 5.4+. Older version may work, but you're on your own.

### Will you be around fixing bugs & maintain this driver ?
I will try, but my knowledge & abilities are very limited. Not to mention I am very busy right now due to spending time on [BlissOS](https://blissos.org/) & focusing most of my time on university's capsule project.

Any contributions to the project especially during these times are like ***✨gold✨*** to me, thank you very much. 🙏

### From the info gathered in this driver, can we add MT7902 support to mt76 driver ?

Yes and No. `Yes` you can and I already did some attempt before. However, `No` because it is not as easy as adding mt7902 info to the driver and expecting it to run smoothly. It turned out that Mediatek gave [a very special firmware to the MT7921 family](https://github.com/tnguy3333/mt7902/issues/7#issuecomment-3263501573) and you will have to spend extra time to figuring out how to use the Windows firmware on the mt76 driver.

### How about Bluetooth ?

I don't know, I never tested Bluetooth before. I did make a patch for `btmtk` based on the info I gathered from [rodin's BSP](https://github.com/MiCode/MTK_kernel_modules/tree/bsp-rodin-v-oss/connectivity/bt/linux_v2). If you planned to build the kernel on your own or making OOT driver for `btmtk`, you can find it here:

https://gist.github.com/hmtheboy154/b2675e02d5f9a0bb861598e77ec2f38f

### I noticed there's this newer [gen4m](https://github.com/MiCode/MTK_kernel_modules/tree/bsp-rodin-v-oss/connectivity/wlan/core/gen4m) driver which looks more active. Can we bring MT7902 info to that driver and use it instead ?

Maybe ? I've tried to compare [gen4m's MT7961](https://github.com/MiCode/MTK_kernel_modules/tree/bsp-rodin-v-oss/connectivity/wlan/core/gen4m/chips/mt7961) to the `gen4-mt79xx` one before and looks like there're a lot of changes. It doesn't look impossible though, maybe you can try :).

### Is there a place like a group chat that we can talk about the development of the driver ?

There's this [Discord group](https://discord.gg/JGhjAxEFhz) that I [found](https://github.com/OnlineLearningTutorials/mt7902_temp/issues/8#issuecomment-2933979855).
