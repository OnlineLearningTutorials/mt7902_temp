# MediaTek MT7902 WiFi 6E Driver for Linux

Out-of-tree driver and firmware for the **MediaTek MT7902 802.11ax (WiFi 6E) PCIe Wireless Network Adapter [Filogic 310]**, based on the mt76 driver stack.

Tested on:
- **ASUS TUF GAMING B650M-E WIFI** motherboard
- **Fedora 43** (kernel 6.18.x)
- Secure Boot enabled

> [!NOTE]
> This is a fork of [OnlineLearningTutorials/mt7902_temp](https://github.com/OnlineLearningTutorials/mt7902_temp) with Fedora-specific fixes, Secure Boot module signing support, and updated instructions.

## Prerequisites

Install kernel development headers:

```
sudo dnf install kernel-devel-$(uname -r)
```

## WiFi Driver

The driver source is in the `latest/` folder.

### Quick Install

```
cd latest
make
```

This will compile, compress, sign (if MOK key exists), and install the modules.

### Step by Step

**Compile:**

```
cd latest
make module_compile
```

**Compress:**

```
make module_compress
```

**Sign (Secure Boot):**

```
make module_sign
```

> [!IMPORTANT]
> If Secure Boot is enabled, unsigned modules will be rejected with `Key was rejected by service`. See the [Secure Boot](#secure-boot) section below to set up module signing.

**Install:**

```
sudo make module_install
```

**Remove stock modules and reload:**

The stock kernel ships its own mt76 modules that don't support the MT7902. You need to remove them so your custom modules take priority:

```
sudo rm /lib/modules/$(uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt76.ko.xz
sudo rm /lib/modules/$(uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt76-connac-lib.ko.xz
sudo rm /lib/modules/$(uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt792x-lib.ko.xz
sudo rm /lib/modules/$(uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt7921/mt7921-common.ko.xz
sudo rm /lib/modules/$(uname -r)/kernel/drivers/net/wireless/mediatek/mt76/mt7921/mt7921e.ko.xz
sudo depmod -a
```

**Load the driver:**

```
sudo modprobe -r mt7921e mt7921_common mt792x_lib mt76_connac_lib mt76
sudo modprobe mt7921e
```

**Verify:**

```
lsmod | grep mt79
sudo dmesg | grep -i mt79
```

## Firmware

Firmware files are in the `mt7902_firmware/` folder.

```
cd mt7902_firmware
sudo bash install_firmware.sh
```

This copies the following to `/lib/firmware/mediatek/`:
- `WIFI_MT7902_patch_mcu_1_1_hdr.bin.zst`
- `WIFI_RAM_CODE_MT7902_1.bin.zst`
- `BT_RAM_CODE_MT7902_1_1_hdr.bin.zst`

## Secure Boot

Fedora ships with Secure Boot enabled by default. Custom-compiled kernel modules must be signed with a Machine Owner Key (MOK) or they will be rejected.

### One-time setup

Generate a signing key and enroll it:

```
cd latest
make mok_setup
```

This will:
1. Generate a key pair at `/root/.mok/MOK.priv` and `/root/.mok/MOK.der`
2. Prompt you to set a password for MOK enrollment

After running this, **reboot**. The MOK Manager (blue screen) will appear:
1. Select **Enroll MOK**
2. Select **Continue**
3. Enter the password you set
4. Select **Reboot**

After this one-time enrollment, the key persists across reboots and kernel updates.

### Signing modules

Once the MOK key is enrolled, `make` and `make module_sign` will automatically sign all compiled modules. You can verify a module is signed:

```
modinfo mt7921e | grep signer
```

## Bluetooth

> [!WARNING]
> If the bluetooth driver conflicts with `gen4-mt7902`, remove its firmware so it doesn't interfere:
> ```
> sudo rm /lib/firmware/mediatek/mt7902/BT_RAM_CODE_MT7902_1_1_hdr.bin.zst
> ```
> This project uses the firmware at `/lib/firmware/mediatek/BT_RAM_CODE_MT7902_1_1_hdr.bin.zst`

To enable bluetooth, go to the directory matching your kernel version (e.g. `./linux-6.18/drivers/bluetooth`), then compile and load:

```
make
sudo rmmod btusb
sudo rmmod btmtk
sudo insmod btmtk.ko
sudo insmod btusb.ko
```

## Troubleshooting

**`modprobe: ERROR: could not insert 'mt7921e': Key was rejected by service`**
Secure Boot is blocking unsigned modules. Follow the [Secure Boot](#secure-boot) section.

**Driver loads but WiFi interface doesn't appear**
Check if the stock `.ko.xz` modules are still present and taking priority over your `.ko.zst` modules:
```
modinfo -F filename mt7921e
```
If it shows a `.ko.xz` path, remove the stock modules as described above.

**`modprobe: ERROR: ... kmod_file_load_zstd() zstd: Src size is incorrect`**
The `.ko.zst` module files are corrupt. This happens if modules were signed *after* zstd compression (the signature bytes appended to the file break the zstd format). Fix: rebuild with `make clean && make` — the Makefile now signs `.ko` files *before* compressing them. Alternatively, install unsigned `.ko` files directly and sign them in place.

**`modprobe: FATAL: Module mt7921e not found`**
Run `sudo depmod -a` to refresh module dependencies.

**`Message 00020001 (seq N) timeout` in dmesg (infinite loop / kernel panic)**
The firmware is incompatible. The `mt7902_firmware/` directory contains two firmware versions — the older files in the root and newer files in `latest/`. The older firmware (Build Time `20220729`) causes the MCU to fail to initialize, producing an infinite timeout loop that can lead to a kernel panic. Use the firmware from `mt7902_firmware/latest/` (Build Time `20251212`). The `install_firmware.sh` script now does this by default.

## Security Audit

A source code audit was performed on the driver and firmware. **No malicious code was found** — no backdoors, no data exfiltration, no hidden network connections, no userspace command execution.

### No Security Concerns Found

- No hardcoded IPs, URLs, or C2 infrastructure
- No `sock_create`, `kernel_connect`, or network socket usage
- No `call_usermodehelper` (can't spawn userspace processes)
- No obfuscated strings or suspicious hex payloads
- Makefile only does standard kernel module compilation + install
- Firmware install script just copies 3 files to `/lib/firmware/mediatek/`

### Known Bugs/Issues

1. **`is_mt7921()` redefined incorrectly** — In `mt76_connac.h`, this function was changed to return true for chip ID `0x7902` instead of `0x7961`. This is a porting bug. If you only have an MT7902 card (no MT7921/7922/7920), it won't affect you, but it shows sloppy porting from upstream.
2. **More than a minimal PCI ID patch** — The changes include DMA layout rework, IRQ map overrides, firmware init logic rewrites, and removal of `NO_VIRTUAL_MONITOR`. These are plausible hardware adaptations but go beyond "just adding a PCI ID."
3. **Two firmware `.bin` files have executable permissions (755)** — Harmless packaging mistake, should be 644.
4. **Binary firmware blobs are inherently unauditable** — This is standard for all WiFi drivers (Intel, Broadcom, etc.), not unique to this project.

### Verdict

The source code is not malicious but is rough around the edges — it's clearly a work-in-progress port. The risk is firmware bugs or crashes, not security compromise.

## Credits

- Original driver development: [OnlineLearningTutorials/mt7902_temp](https://github.com/OnlineLearningTutorials/mt7902_temp)
- Alternative driver: [hmtheboy154/gen4-mt7902](https://github.com/hmtheboy154/gen4-mt7902)
- MediaTek mt76 upstream: [openwrt/mt76](https://github.com/openwrt/mt76)

## License

ISC (same as upstream mt76)
