# ps5-linux

**ps5-linux** turns a supported PS5 **Phat or Slim** into a Linux PC by using the console's supported Hypervisor path. It provides access to the CPU, GPU, USB, storage, network and optical-drive hardware.

Features:

- HDMI 4K60 video and audio output
- M.2 SSD as a dedicated Linux device
- All USB ports available for peripherals
- BD drive support through a custom AHCI driver
- Internal Bluetooth support through a custom XHCI driver
- Ethernet support through a custom GBE driver

![Alt Text](logo.webp)

## PS5 firmware

*ps5-linux* currently supports PS5 Phat and Slim on the following firmware versions:

- **3.00**, **3.10**, **3.20**, **3.21** — no M.2 support
- **4.00**, **4.02**, **4.03**, **4.50**, **4.51** — M.2 support
- **5.00**, **5.02**, **5.10**, **5.50** — M.2 support
- **6.00**, **6.02**, **6.50** — M.2 support
- **7.00**, **7.01**, **7.20**, **7.40**, **7.60**, **7.61** — M.2 support

Support for 1.xx and 2.xx is not currently planned.

### 13.60 port status

This fork contains an **experimental 13.60 kernel profile**. Firmware 13.60 is detected and the loader uses the verified `VMSPACE_VM_PMAP = 0x2e8` value. Public 13.60 sources also corroborate the kernel text/data layout and additional kernel metadata.

The Linux boot path remains **fail-closed** on 13.60. The loader now has an executable diagnostic path that validates the 13.60 kernel profile and kernel R/W environment, then exits without Linux file mapping, resume preparation or the rest-mode handoff.

This makes the 13.60 payload **executable and testable on-console**, but it is **not a Linux boot yet** because a compatible, independently verified 13.60 Linux HV backend is still missing.

For firmware updates, use the correct PUP and follow Sony's [official system-software procedure](https://www.playstation.com/en-us/support/hardware/reinstall-playstation-system-software-safe-mode). There is no official PS5 downgrade path.

## Hardwares

To run *ps5-linux*, you need:

- **Required**: USB drive of at least 64 GB; an external SSD is strongly recommended.
- **Required**: USB keyboard and mouse; wireless dongles are supported.
- *Optional*: USB WLAN adapter.
- *Optional*: PS5-compatible M.2 SSD for Linux.
- *Optional*: Bluetooth dongle for a DualSense controller.

## Configure PS5 settings

- **VERY IMPORTANT**: Enable `Settings` → `System` → `Power Saving` → `Features Available in Rest Mode` → `Supply Power to USB Ports` → `Always`.
- **VERY IMPORTANT**: Disable `Settings` → `HDMI` → `Enable HDMI Device Link`.
- *Recommended*: Disable automatic system-software updates.
- *Recommended*: Disable automatic system-error reporting.

Reapply these settings after resetting the PS5 or reinstalling firmware.

## Installation

### 1. Get a Linux image

#### Pre-built images

Download a pre-built image from [ps5-linux-image](https://github.com/ps5-linux/ps5-linux-image/releases/tag/latest). The recommended image is `ps5-ubuntu2604.img.xz`; unpack the archive before flashing.

#### Build your own image

On Windows, install WSL from an Administrator PowerShell or Command Prompt:

```bash
wsl --install
```

Install Docker:

```bash
sudo apt update
sudo apt install docker.io -y
sudo service docker start
sudo usermod -aG docker $USER
```

Clone and build the image:

```bash
cd ~/
git clone https://github.com/ps5-linux/ps5-linux-image
cd ps5-linux-image
chmod +x ./build_image.sh
./build_image.sh --distro ubuntu2604
```

The resulting image is `output/ps5-ubuntu2604.img`.

### 2. Flash the image to USB

Use a drive of at least 64 GB. An external SSD is strongly recommended.

#### Linux/macOS:

```bash
# Check the target device first: lsblk / diskutil list
sudo dd if=output/ps5-ubuntu2604.img of=/dev/sdX bs=4M status=progress conv=fsync
```

#### Windows (Balena Etcher):

Download [Balena Etcher](https://etcher.balena.io/), select the `.img` file, select the USB drive and flash it.

### 3. Plug the USB drive into your PS5

Supported boot ports:

- Front bottom USB Type-C
- Rear USB Type-A ports

The front top Type-A port is USB 2.0 and is not recommended.

### 4. Run the jailbreak

Use the jailbreak method appropriate for your PS5 firmware and follow its current documentation. This repository does not duplicate firmware-specific exploit instructions here.

### 5. Send the payload

On ARM64 Linux, install the x86-64 cross-compilation tools if you need to build the payload:

```bash
sudo apt install gcc-x86-64-linux-gnu binutils-x86-64-linux-gnu
```

Build with [ps5-payload-sdk](https://github.com/ps5-payload-dev/sdk):

```bash
git clone https://github.com/ps5-linux/ps5-linux-loader
cd ps5-linux-loader
make
```

For this fork, check out the `develop/13.60` branch when testing the 13.60 port.

On supported firmware, the payload prepares the Linux resume environment and enters rest mode. Wait for the orange LED to become static before powering the console back on.

If the display remains black after a successful Linux handoff, try removing the relevant `video=` entry from `cmdline.txt`, testing HDCP in both states, trying another display, or enabling `amdgpu.force_1080p=1`.

Report persistent issues in the [Discord server](https://discord.gg/PeMGVB7BAm) with the monitor EDID.

## First Boot

Configure the system and keep your login password safe.

Recommended first steps:

1. Disable the screen saver; it is currently buggy.
2. Reconnect the wired or WLAN interface if networking is not immediately available.
3. Hold the kernel packages before running `apt upgrade`.
4. Install Firefox if needed.
5. Update Mesa using the recommended PS5/AMD procedure.
6. Build [ps5-linux-tools](https://github.com/ps5-linux/ps5-linux-tools).
7. If your console has a supported Marvell WLAN chipset, install the matching driver.

## M.2 installation

An M.2 SSD can be dedicated to Linux; it cannot simultaneously be used for PS5 game storage.

1. Install the M.2 SSD using the [official guide](https://www.playstation.com/en-us/support/hardware/ps5-install-m2-ssd).
2. **VERY IMPORTANT**: Reformat an M.2 previously used for PS5 games from `Settings` → `Storage` → `M.2 SSD Storage` before using it for Linux.
3. Boot Linux and initialize the SSD with `ps5-linux-tools`.
4. Reboot and verify that the M.2 remains available.
5. Install the Linux image to the M.2.
6. Boot Linux from the M.2 using the corresponding `ps5-linux-tools` helper.

To boot from M.2 by default, update the root label in `/boot/efi/cmdline.txt`. The USB FAT32 partition is still required.

## Fan & boost control

Use [ps5-linux-tools](https://github.com/ps5-linux/ps5-linux-tools) to control the fan curve and enable the CPU/GPU boost configuration.

```bash
cd ps5-linux-tools
sudo ./ps5_control --fan on
sudo ./ps5_control --boost on
```

Always enable the fan before boost, matching the behavior of the PS5 OS.

## Updating ps5-linux

For future *ps5-linux* updates, download the matching `.deb` or `.pkg.tar.zst` packages from [ps5-linux-patches](https://github.com/ps5-linux/ps5-linux-patches/releases) and install them normally.

## FAQ

- Q: Will firmware >=8.00 be supported?
  - A: Upstream does not support it. This fork has an experimental 13.60 kernel profile, but 13.60 Linux boot is still blocked by the missing HV backend.
- Q: Why can I not use M.2 on 3.xx?
  - A: The PS5 does not boot with an M.2 device attached on those firmware versions.
- Q: Can I dual-boot Linux and PS5 OS?
  - A: No. This is a soft-mod; the console must be prepared again before booting Linux.
- Q: Can I put Linux into standby and resume?
  - A: No. Standby/resume is not supported.
- Q: Can I continue using my PS5 after installing Linux?
  - A: Yes. The internal PS5 storage is not modified by the Linux setup.
- Q: Can I use the PS5 NIC/WLAN module in Linux?
  - A: Ethernet is supported on all models. WLAN currently requires a supported Marvell chipset.
- Q: Does the DualSense controller work?
  - A: Yes, through internal Bluetooth or a Bluetooth dongle.
- Q: What resolutions and refresh rates are supported?
  - A: 1080p, 1440p and 2160p at 60 Hz are broadly supported. 1440p@120 Hz is confirmed on the DELL S3225QC; other high-refresh combinations are less broadly tested.
- Q: After reboot, I see “Repairing” and “Your PS5 wasn't turned off properly.” Is that normal?
  - A: Yes. This can occur after the Linux boot flow and is harmless.

## Tips and tricks

- For graphical issues in games, try `RADV_DEBUG=nohiz`.
- Kernel parameters can be edited in `cmdline.txt` on the FAT32 partition.
- VRAM can be adjusted in `vram.txt`; the default is 512 MB (`0x20000000`).
- Monitor hotplug may work, but it does not automatically change resolution.
- Some displays have problems when `video=DP-1:` is present in `cmdline.txt`.

Many settings and troubleshooting techniques from the [AMD BC250 Documentation](https://elektricm.github.io/amd-bc250-docs/) also apply to PS5.

## Bugs

- Screen saver does not work properly.
- HDMI audio does not work on some monitors.
- HDMI 1440p and 2160p output does not work on some monitors.

## Upstreamed changes

During the project, we upstreamed:

- [drm/amd: fix dcn 2.01 check](https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/drivers/gpu/drm/amd/display/dc?id=39f44f54afa58661ec9c27e15f5dbce2372892)
- [amd/addrlib: Add more GFX1013 GPUs](https://gitlab.freedesktop.org/mesa/mesa/-/commit/44bed00b8bbcb1825e2c920cf1a828efdc72b1f1)

## Discord

Join the [Discord server](https://discord.gg/PeMGVB7BAm) for Linux-on-PS5 news, troubleshooting, development, tips and issue reports.

## Credits

- [theflow](https://github.com/TheOfficialFloW): [ps5-linux-loader](https://github.com/ps5-linux/ps5-linux-loader), [ps5-linux-patches](https://github.com/ps5-linux/ps5-linux-patches), [ps5-linux-tools](https://github.com/ps5-linux/ps5-linux-tools)
- [c0w](https://github.com/c0w-ar): [ps5-linux-loader](https://github.com/ps5-linux/ps5-linux-loader)
- [resulknad](https://github.com/resulknad): [ps5-linux-image](https://github.com/ps5-linux/ps5-linux-image)
- [rmuxnet](https://github.com/rmuxnet): [PS5 Ethernet driver](https://github.com/ps5-linux/ps5-linux-patches/commit/643e214d7bd37f292045fc0dbb821e421f7a3e47)
- [fail0verflow](https://github.com/fail0verflow): [prosperous](https://github.com/fail0verflow/prosperous)
- [flatz](https://github.com/flatz): [HV exploit](https://gist.github.com/flatz/620ddda6d64acca6d1c990dc3080ac0e)
- [cragson](https://github.com/cragson): [HV exploit implementation](https://github.com/cragson/ps5-hen)
- [john-tornblom](https://github.com/john-tornblom): [PS5 SDK](https://github.com/ps5-payload-dev/sdk)
- [echostretch](https://github.com/echostretch): Offsets and testing
- [kirathenotebook](https://github.com/kirathenotebook): Betatesting and README contribution
- 15432: Tests on BC-250