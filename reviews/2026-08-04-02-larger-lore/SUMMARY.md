# Summary

Version 0.1.3 makes temporary lore and action-result messages materially easier
to read without enlarging ordinary selected-action descriptions.

- Timed lore uses Pebble's supported Gothic 24 system font.
- The timer increases from 2.2 to 4.2 seconds, exactly two seconds longer.
- Emery temporarily shows two action rows to reserve more lore space.
- Basalt temporarily hides the compact resource status and moves the selected
  action upward, reserving a 90-pixel lore area without overlap.

Host tests, the SDK 4.17 Basalt/Emery build, exact emulator installs, and live
layout review passed. The exact reviewed PBW installed on the physical Time 2
with exit code 0 and `App install succeeded`; hands-on readability remains for
hc to confirm.
