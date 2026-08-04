# Summary

Version 0.1.7 makes Feed Hearth a two-second timed action. Search Rooms and
Feed Hearth now share the same timer, progress bar, 100 ms updates, and input
lock. Feed validity is checked before starting; kindling and Fire change only
once, after the bar completes, and the completed state is then saved.

Emery and Basalt emulator runs show `Feeding hearth...` without layout overlap.
Before/progress/after evidence confirms that one feed spends two kindling and
raises Fire by one only after completion. Host tests and the universal build
passed. The Time 2 CloudPebble install reported `App install succeeded`;
physical timing feel remains for the wearer to validate.

During review, Emery screenshot transport repeatedly timed out while QEMU was
still alive. The emulator-only flash was moved to the recoverable backup
`qemu_spi_flash.bin.backup-20260804-1921`; a clean relaunch restored capture.
No project data or physical watch save was reset.
