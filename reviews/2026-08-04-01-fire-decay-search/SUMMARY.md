# Summary

Version 0.1.2 implements both requested timing changes.

- Fire loses one point per accumulated 60 seconds until it reaches zero.
- Feeding costs two kindling, adds one Fire, and resets the partial-minute
  decay counter.
- Timestamp catch-up applies the same decay after the app is closed.
- Schema-1 saves migrate in place to schema 2 without changing record size.
- Search Rooms shows a two-second progress bar, blocks navigation and duplicate
  activation during the action, then grants exactly one search reward.

Host state tests, the SDK 4.17 Basalt/Emery build, and live emulator review
passed. The exact reviewed PBW installed on the physical Time 2 with exit code
0 and `App install succeeded`; hands-on timing and feel remain for hc to
confirm. Physical Time Steel remains unverified for this build.
