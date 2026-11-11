# DS Style v8.1 Blue Test 3

## Before installing

**Hardware test only.** No physical EZODE-B has been tested yet. A failed
flash or storage operation could require recovery or cause data loss. Use
a spare SD card with disposable games and saves. Do not test NOR changes
on a cart with anything in NOR that you cannot replace.

- Confirm the PCB is blue and marked **EZODE-B**. The earlier Omega
  Definitive Edition Rev B is a different cartridge.
- In Mode A, confirm the official Blue kernel **1.02 / firmware 3** works.
  Use the [EZ-FLASH download page](https://www.ezflash.cn/download/) or the
  [Blue-specific download page](https://www.ezflashomega.com/pages/EZ-Flash-Omega-Definitive-Edition-B-Downloads.html).
  Install or update the firmware with the **official Blue kernel** before
  testing. DS Style does not update the FPGA firmware.
- Back up the entire SD card. Boot the stock kernel once to finish any
  pending save backup. A PC copy of the card does not back up the NOR.
- Keep the official `ezbluekernel.bin` on a separate SD card for rollback.
  For official K1.02/FW3, the published SHA-1 is
  `ff66ff0413dea3715023c8f04e51faaf96b87014`.

## Install and test

1. Extract the test ZIP to the root of the spare SD card. You should see
   `ezbluekernel.bin` and `SYSTEM` side by side. The original Omega and
   older DE kernel files are not interchangeable with this one.
2. With the console off, insert the SD card and the cartridge in Mode A.
   Hold **R** while powering on. Use a reliable power source and do not
   switch off or remove the card during the update.
3. First check cold boot, SD browsing and **Clean boot** with a small GBA
   game. Then check a disposable battery save: save, return or reboot,
   relaunch and confirm the save is still there.
4. For a GBA game launched **with add-ons**, try **Start + Select** and
   **Exit game**, then relaunch and hold **Select** for about three seconds.
   Try the equivalent controls in GB and GBC. Record your console, stock
   firmware, game, launch mode, and what you see if anything fails.
5. Only after basic tests pass, try NOR with disposable content. Press
   **Start** in NOR to see usage. Test writing several small games, deleting
   the middle entry, and writing again into the freed space. If the NOR
   is factory-erased, the test recognises it as empty. It still refuses to
   write if the allocation table and game records disagree. Do not format
   NOR to work around this on a cart containing anything you want to keep.
6. Open a `.txt`, `.md` and `.markdown` file. Check that the correct file
   opens and that **B** returns to DS Style. Optional Blue-compatible
   PogoShell plugins go in `SYSTEM/PLUG/BLUE`, named for the file extension.
   Do not copy your old cartridge's plugin directory into this folder.
7. Under **Settings > Interface > Load style**, try preparing a copy of
   this test's `ezbluekernel.bin` from `SYSTEM/KERNELS`. Also verify that a
   kernel for an older cart is rejected. The previous SD kernel file is
   retained as `ezbluekernel.bak`; it is not an automatic firmware rollback.
   If the card is removed during preparation and the root kernel file is
   missing, restore it from your PC backup before holding R to update.
8. For a new disposable NOR game written **with add-ons** and **Save states**
   enabled, try Save state, Load state, then **Exit game**. Return all the
   way to the launcher before switching off. Relaunch the same NOR entry
   and check that its state loads. The state is saved to the SD card only
   when you exit the game back to DS Style. Test this in **Mode A**.

To try an existing battery save, **copy** that save from stock's root
`SAVER` folder into `SYSTEM/SAVER` on the test card. Keep the original.
Do not move or import stock save states into the test.

## Limitations

- Older NOR entries with embedded SD sector addresses are not launched by
  this test. Keep them intact and use the official Blue kernel for them,
  or rewrite a disposable copy with this build to test the new save-state
  path. NOR states are separate from SD-game and stock states. Before going
  back to stock, remove the test add-on entries written to NOR if you do not
  need them; reinstalling stock does not remove them.
- ROMs over 32 MiB are clean-boot only and must use Blue's supported
  large-ROM format. Native NES uses the Blue stock emulator.
- Load style accepts checksummed Blue builds starting with Test 3. Older
  test builds lack the checksum and are not accepted by Load style. The
  current Customiser and browser installer are not for Blue. Third-party
  plugins may still need their exit code adapted for Blue.
- Hardware verification is still needed for install and rollback, FRAM,
  Mode B, 64 MiB ROMs, NOR writes and the physical warm-return path.

If a battery-save backup fails, launches and manual FRAM transfers are
blocked to protect the pending save. Correct the SD problem and restart
with the same cartridge and card to retry. Do not discard the pending save
unless you intend to lose that progress.

## Report a problem

In a [GitHub issue](https://github.com/FrankieT19/omega-blue-ds-style-kernel/issues),
include the test version, console, stock firmware, SD format, and the steps
to reproduce it. For games, note SD or NOR, clean boot or add-ons, the game
title, and whether you can reproduce it on stock. For plugins, include the
file extension and whether the bundled viewer or a third-party plugin was
used. Do not attach ROMs or personal saves.

## Uninstall or recover

1. Power off after saving and allowing any pending save backup to finish.
   Back up the test card, including new files under `SYSTEM/SAVER`.
2. Insert the card with the **official Blue** `ezbluekernel.bin` at its
   root. Set Mode A and power on while holding **R**, just as for a stock
   kernel update. Do not interrupt the update. This restores the kernel;
   it does not undo NOR changes or downgrade FPGA firmware.
3. Return to your original SD card or restore its backup. To keep test
   progress, copy only the wanted battery saves from `SYSTEM/SAVER` into
   stock's root `SAVER` folder. Back up existing files before replacing
   any of them. Leave the test's RTS and patch files out of the stock setup.
4. If the official Blue recovery update does not start or cannot finish,
   stop and contact EZ-FLASH. Do not try an older Omega, DE Rev A/Rev B or
   other model's bootloader, firmware or recovery file.
