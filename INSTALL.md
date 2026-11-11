# DS Style v8.1 Blue Test 5

**Hardware test for EZODE-B.** Back up your SD card. Use disposable games and saves for NOR testing.

**Install:** Extract the ZIP to the SD root: `ezbluekernel.bin` and `SYSTEM/` go side by side. Games can stay where they are. To use a stock save, copy it from `SAVER/` to `SYSTEM/SAVER/`. In Mode A, hold **R** while powering on. Do not interrupt flashing. If you need a firmware update, use the official Blue kernel separately; DS Style does not update firmware.

**Test:** Try rapid scrolling in both carousels and List + Art, SD boot and saves, large ROMs with add-ons, and NOR write/delete and save states. For new NOR states, exit to DS Style before powering off. For **Load style**, put a copy of the Test 5 kernel in `SYSTEM/KERNELS/`.

**Uninstall:** Put the [official Blue `ezbluekernel.bin`](https://www.ezflashomega.com/pages/EZ-Flash-Omega-Definitive-Edition-B-Downloads.html) at the SD root, replacing the test file. Flash with **R** and restore your SD backup. Reflashing does not undo NOR changes.

[Report an issue](https://github.com/FrankieT19/omega-blue-ds-style-kernel/issues) with your firmware version and steps to reproduce.
