# Changelog

## v8.2 Blue Test 7

- Added rebindable in-game menu and exit shortcuts.

## v8.1 Blue Test 6

- Word-align the animation workspace used by both carousels.
- Check sampled ROM data after the fast SD copy and retry through the checked reader on mismatch.
- Keep launch interrupts in IWRAM and stop on failed SD reads before patching or booting.

## v8.1 Blue Test 5

- Restore the original DS Style bitmap assets and DMA drawing paths.
- Remove the FPGA firmware payload and updater from DS Style.

## v8.1 Blue Test 4

- Allow large-ROM add-ons on SD and NOR when the patch and scratch space fit.
- Restore the stock Blue firmware update prompt and graphical NOR usage map.
- Support stock NOR save-state entries when the original sector map matches.
- Store the DS Style launch preference separately from Blue's Mode B setting.

## v8.1 Blue Test 3

- Protect a pending battery save from being overwritten if its backup fails.
- Reject empty or unsigned SD save states before restoring game memory.
- Include the add-on sector when it falls beyond the ROM data in a NOR write.
- Recognise factory-erased NOR metadata without writing during browsing.
- Verify the complete kernel payload before staging a style.

## v8.1 Blue Test 2

- Added PogoShell loading and bundled the Blue text and Markdown viewer.
- Enabled Load style for DS Style images built for the Blue layout.
- Added experimental NOR save states for newly written add-on entries.
  Save states are written to the card on return to DS Style, not by writing
  to hard-coded SD sectors from the game.
- Enabled the built-in Blue NES emulator when no NES plugin is present.

## v8.1 Blue Test 1

- Initial hardware-test port for the EZ-FLASH Omega Definitive Edition B.
- DS Style launcher, animated artwork views, and light and dark themes.
- Blue-specific 120 MiB NOR allocator, per-game deletion and usage view.
- Blue large-ROM flag and native SD, FRAM, firmware and flash layout.
- DS Style GBA add-on popup and Select-hold exit, plus borderless GB/GBC.
- NOR record and bounds checks; NOR save-state add-ons disabled in this test.

See [INSTALL.md](INSTALL.md) for the hardware warning and current limits.
