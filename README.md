# DS Style for EZ-FLASH Omega Definitive Edition B

DS Style is a Nintendo DS-inspired kernel for the EZ-FLASH Omega family.
This repository is for the **blue PCB marked EZODE-B** only. It is not for
the earlier Omega Definitive Edition Rev A or Rev B, or the original Omega.

**v8.1 Blue Test 3 is experimental and has not yet been verified on a
physical EZODE-B.** Use a spare SD card, back up your saves and NOR data, and
keep the official Blue kernel on a separate card before installing.

- [Download Blue Test 3](https://github.com/FrankieT19/omega-blue-ds-style-kernel/releases/tag/v8.1-blue-test.3)
- [Installation, testing and uninstallation](INSTALL.md)
- [DS Style project page](https://frankiet19.github.io/omega-de-ds-style-kernel/project/)
- [Original Omega Definitive Edition](https://github.com/FrankieT19/omega-de-ds-style-kernel)
- [Original Omega](https://github.com/FrankieT19/omega-ds-style-kernel)

The Blue port uses the Blue mapper, firmware layout, save handling, and NOR
allocation scheme from EZ-FLASH. The DS Style launcher, artwork views and
animations are based on v8.1. See [the changelog](CHANGELOG.md) for the test
build's scope and limitations.

## Build

Install devkitPro with devkitARM, libgba and gbafix, and Python 3. On
Windows the build defaults to `C:/devkitPro`; elsewhere set `DEVKITPRO`
or pass `--devkitpro`. The build checks the devkitARM CRT before installing
the warm-return trampoline and rejects any image that would overlap the
vendor firmware. The default output is named `blue-upgrade-layout.test`.

```sh
python tools/build.py
```

Use `--flashable` to name the checked output `build/ezbluekernel.bin` for a
physical Blue hardware test. Do not run the original stock link step on
this payload: it would overwrite the part of the port above 1 MiB.
The same command builds `build/txt.bin`, the Blue text viewer. Install
copies under `SYSTEM/PLUG/BLUE` as `txt.bin`, `md.bin` and `markdown.bin`.

## Credits

DS Style by FrankieT19. Original Blue and Omega kernel source by EZ-FLASH.
PogoShell integration in DS Style was inherited from Simple by Sterophonick.
Thai language support by aidiadayo. Contributions by Merkin and tindmackey.
In-game return work is adapted from Slot. Advance. See [Notices](NOTICE.md)
for source and licence details.
