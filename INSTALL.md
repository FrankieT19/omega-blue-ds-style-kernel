# DS Style v8.1 Blue Test 4

**Hardware test for EZODE-B.** Back up your SD card. Use disposable games and saves for NOR testing.

**Install:** Extract the ZIP to the SD root: `ezbluekernel.bin` and `SYSTEM/` go side by side. Games can stay where they are. To try an existing stock save, copy it from `SAVER/` to `SYSTEM/SAVER/`. In Mode A, hold **R** while powering on. Do not interrupt flashing. On firmware below 3, the kernel offers the stock firmware update.

**Test:** Try SD boot and saves, **Start + Select** and **Exit game**, large ROMs with add-ons on SD and NOR, NOR write/delete and save states, Mode B, and the text viewer. For **Load style**, put a copy of the test kernel in `SYSTEM/KERNELS/`. For new NOR states, exit to DS Style before powering off. For stock NOR states, leave the original `RTS/` files in place. Large ROMs need room for the patch and, on SD, the add-on scratch data.

**Uninstall:** Put the [official Blue `ezbluekernel.bin`](https://www.ezflashomega.com/pages/EZ-Flash-Omega-Definitive-Edition-B-Downloads.html) at the SD root, replacing the test file. Flash with **R** and restore your SD backup. Reflashing does not undo NOR changes.

[Report an issue](https://github.com/FrankieT19/omega-blue-ds-style-kernel/issues) with your firmware version and steps to reproduce.
