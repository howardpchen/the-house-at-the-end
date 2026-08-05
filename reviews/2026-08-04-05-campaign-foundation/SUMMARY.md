# Summary

Version 0.2.0 changes the proven vertical slice into a campaign-path alpha.
It adds a deterministic 31×31 Drift, a 7×7 button-driven map, bounded compiled
scenes, movement progression, three endings, and a transactional two-bank save
that migrates 0.1.x records.

## Measured result

- Universal PBW: 59,969 bytes.
- PBW SHA-256:
  `42d27feeadc734de4aa8f656a9ef6990e6ca266a40eb063f870d95dbda464a1c`.
- Resources: 8,713 bytes per platform.
- Application RAM footprint: 20,664 bytes.
- Free Basalt heap: 44,872 bytes.
- Free Emery heap: 110,408 bytes.
- Compiled content: 20 scenes, 73 strings, 573 bytecode bytes, 4,032 string
  bytes, longest page 80 characters.
- Redundant persistent state: approximately 0.7 KB.
- Exact reviewed PBW installed successfully on physical Time 2 through
  CloudPebble; migration behavior and interaction feel remain wearer tests.

## Scope boundary

The narrative route through Movements II–V and Wake, Keep, and Become the Door
is implemented and host-tested. This is not the content-complete beta. The full
facility economy, equipment tiers, encounter catalog, guest conversation arcs,
Chronicle variants, ending-quality variants, optional finale interaction, and
release balance remain to be built and tested.

## Review note

Immediate screenshots after a timer-driven notice clear or a rapid view change
occasionally captured an incomplete emulator frame. The next directional input
redrew the complete frame, and canonical screenshots show the correct result.
This may be screenshot/emulator transport behavior, but it remains an explicit
follow-up item rather than being silently discarded.
