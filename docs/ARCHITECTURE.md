# Architecture

This file documents the implemented vertical-slice architecture. The planned
world generator, segmented save, event interpreter, module boundaries, and
memory gates are specified in the spoiler-heavy
[`DESIGN_BIBLE.md`](DESIGN_BIBLE.md).

## Targets

- Pebble Time Steel (`basalt`), 144x168 rectangular color display.
- Pebble Time 2 (`emery`), 200x228 rectangular color display.
- Native C under Pebble SDK 4.17, with SDK compatibility declared as version 3.

Basalt defines the minimum layout. Emery receives proportional spacing and
additional visible copy, but no exclusive control or required information.

## Boundaries

- `house_state`: platform-independent rules, elapsed-time production, unlocks,
  construction, assignments, and expedition resolution.
- `main`: Pebble lifecycle, persistence, navigation, button input, and drawing.
- `tests`: ordinary host-C tests for deterministic state transitions.

The state module intentionally does not include `pebble.h`, allowing its rules
to be tested without an emulator.

## Save record

Pebble persistent storage allows 4 kB per application and 256 bytes per value.
The game uses one compact record containing:

- schema version and record size;
- complete `HouseState` payload;
- checksum over the preceding bytes.

Unknown, truncated, or corrupt records fall back to a new game. Schema changes
must add explicit migration before the version number advances.

Schema 3 reinterprets the existing search counter as a modulo-three shared
gathering counter. Schema-1 and schema-2 records migrate in place without
changing the 40-byte `HouseState` size.

## Elapsed time

The save timestamp is the source of truth. On launch and during foreground
ticks, elapsed seconds are converted into gatherer/listener production and
one-point-per-two-minutes hearth decay. Feeding the hearth resets its partial
two-minute counter. Catch-up is capped at six hours. Guest production accrues
only for the portion of elapsed time during which Fire remains Seen or warmer.
Negative elapsed time is ignored. No background worker, wakeup event, phone
service, or network connection is required. Schema 2 reuses the schema-1 state
padding byte for the hearth timer, so existing version-1 saves migrate without
changing the record size.

The pure rule layer enforces Fire thresholds rather than relying on disabled UI
rows: Seen (2) enables guest production, Held (3) enables ordinary construction
and ration preparation, and Shared (5) enables guest reassignment, anchor-line
construction, and new expeditions. Existing structures and an active expedition
persist below their thresholds.

## Testing menu

Holding SELECT for one second opens a clearly labeled testing menu. Resource
edits use ten-unit steps; Fire, guest count, and gatherer count use one-unit
steps. Every edit is clamped to normal state limits and persisted immediately.
Changing gatherer count derives listener count so guest assignments remain
valid. Reset requires entering a separate confirmation screen and pressing
SELECT again; BACK cancels.

## Review acceptance

- Both Basalt and Emery compile into the universal PBW.
- The opening and densest unlocked screens fit at 144x168.
- Every action is reachable through physical buttons.
- BACK returns from subsections and exits only from the house hub.
- A fresh save can reach and complete the first memory.
- State survives app restart and rejects a corrupt record.
- Host rules tests pass with warnings treated as errors.
- Emulator evidence is captured separately for Basalt and Emery.
- Physical hardware behavior remains unconfirmed until installed and exercised
  on the corresponding watches.
