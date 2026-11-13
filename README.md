# DS Style for EZ-FLASH Omega Definitive Edition B

DS Style is a Nintendo DS-inspired kernel for the EZ-FLASH Omega family.
This repository is for the **blue PCB marked EZODE-B** only. It is not for
the earlier Omega Definitive Edition Rev A or Rev B, or the original Omega.

**v8.2 Blue Test 7 is experimental.** This build adds configurable in-game shortcuts. The Blue port still needs hardware testing. Use a spare SD card or back up your data.

- [Download Blue Test 7](https://github.com/FrankieT19/omega-blue-ds-style-kernel/releases/tag/v8.2-blue-test.7)
- [Installation, testing and uninstallation](INSTALL.md)
- [DS Style project page](https://frankiet19.github.io/omega-de-ds-style-kernel/project/)
- [Original Omega Definitive Edition](https://github.com/FrankieT19/omega-de-ds-style-kernel)
- [Original Omega](https://github.com/FrankieT19/omega-ds-style-kernel)

The Blue port uses the Blue mapper, firmware layout, save handling, and NOR
allocation scheme from EZ-FLASH. The DS Style launcher, artwork views and
animations are based on DS Style v8.2. See [the changelog](CHANGELOG.md) for the test
build's scope and limitations.

## Build

Install devkitPro with devkitARM, libgba and gbafix, and Python 3. On
Windows the build defaults to `C:/devkitPro`; elsewhere set `DEVKITPRO`
or pass `--devkitpro`. The build checks the devkitARM CRT before installing
the warm-return trampoline and enforces the Blue upgrade image size.
The default output is named `blue-upgrade-layout.test`.

```sh
python tools/build.py
```

Use `--flashable` to name the checked output `build/ezbluekernel.bin` for a
physical Blue hardware test. Do not run the stock link step on this payload.
Firmware updates are handled separately by the official Blue kernel.
The same command builds `build/txt.bin`, the Blue text viewer. Install
copies under `SYSTEM/PLUG/BLUE` as `txt.bin`, `md.bin` and `markdown.bin`.

## Credits

DS Style by FrankieT19. Original Blue and Omega kernel source by EZ-FLASH.
PogoShell integration in DS Style was inherited from Simple by Sterophonick.
Thai language support by aidiadayo. Contributions by Merkin and tindmackey.
See [LICENSE](LICENSE) and [GPL-3.0](licenses/GPL-3.0.txt). The source for
the bundled GB/GBC emulator is in `emusrc/ds-style`.
