# DS Style v8.1 Blue Test 6

**Experimental, for EZODE-B only.** Use a spare SD card or back up your card and saves first.

**Install:** Copy the contents of `SD Card` to your SD root: `ezbluekernel.bin` and `SYSTEM/` go side by side. In Mode A, hold **R** while powering on. Do not interrupt flashing. To test a stock save, copy it from `SAVER/` to `SYSTEM/SAVER/`.

**Test:** With Animations on, move both ways in the horizontal and vertical carousels on SD and NOR. Then Clean boot the same SD game several times and load a copy of your in-game save. Try Boot with add-ons too. Report any remaining failures, including the game and launch option used.

**What changed:** Fixed carousel memory alignment. SD launches now check samples of the loaded ROM and retry through a checked reader if those samples don't match. NOR writing and game saves are unchanged. No firmware updater is included.

**Uninstall:** Put the [official Blue `ezbluekernel.bin`](https://www.ezflashomega.com/pages/EZ-Flash-Omega-Definitive-Edition-B-Downloads.html) at the SD root, replacing the test file. Flash with **R** and restore your SD backup. Reflashing does not undo NOR changes.

[Report an issue](https://github.com/FrankieT19/omega-blue-ds-style-kernel/issues) with your firmware version and steps to reproduce.
