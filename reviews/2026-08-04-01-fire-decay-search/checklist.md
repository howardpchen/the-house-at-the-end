# Acceptance Checklist

- [x] Fire loses one point after 60 accumulated seconds.
- [x] Fire never decays below zero.
- [x] Feeding resets the partial decay minute.
- [x] Closed-app elapsed time decays Fire on relaunch.
- [x] Existing schema-1 emulator state migrates without a reset.
- [x] Search reward is delayed until the two-second action completes.
- [x] Search displays a visible progress bar on Emery.
- [x] Search displays a visible progress bar on Basalt.
- [x] Input is locked during search to prevent navigation or duplicate rewards.
- [x] Host rules tests pass.
- [x] Universal Basalt/Emery PBW builds and exists.
- [x] Emulator installs report `App install succeeded` for both platforms.
- [ ] Physical Time Steel validation completed for version 0.1.2.
- [x] Physical Time 2 install reports exit code 0 and `App install succeeded`.
- [ ] Physical Time 2 search timing and feel confirmed by hc.
