# Changelog

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
