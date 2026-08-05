# Architecture

This file documents the implemented version 0.2.0 campaign foundation. The
full campaign rules, remaining module boundaries, and memory gates are in the
spoiler-heavy
[`DESIGN_BIBLE.md`](DESIGN_BIBLE.md).

## Targets

- Pebble Time Steel (`basalt`), 144x168 rectangular color display.
- Pebble Time 2 (`emery`), 200x228 rectangular color display.
- Native C under Pebble SDK 4.17, with SDK compatibility declared as version 3.

Basalt defines the minimum layout. Emery receives proportional spacing and
additional visible copy, but no exclusive control or required information.

## Boundaries

- `house_state`: platform-independent rules, elapsed-time production, unlocks,
  construction, assignments, and the Movement I expedition.
- `game_state`: campaign state, movement gates, facilities, named guests, and
  ending reachability.
- `world_gen`: stable 31×31 terrain, movement-banded landmark placement, a
  121-byte visibility mask, and cleared-landmark bits.
- `expedition`: Drift preparation, supplies, movement, hazards, cargo, and
  return rules.
- `scene_vm` and `content_format`: bounded bytecode execution and binary scene
  and string lookup.
- `save_store`: checksummed, segmented, two-bank commits over an abstract
  persistence backend.
- `main`: Pebble lifecycle, backend adapter, navigation, resource reads, button
  input, and drawing.
- `tools/compile_scenes.py`: validates and compiles human-readable JSON into
  Pebble raw resources and a review report.
- `tests`: ordinary host-C tests for deterministic state transitions.

The state module intentionally does not include `pebble.h`, allowing its rules
to be tested without an emulator.

## Save store

Pebble persistent storage allows 4 kB per application and 256 bytes per value.
Version 0.2.0 writes six actual-length segments to an inactive bank:

- core house state;
- named guests;
- world seed, visibility, and cleared landmarks;
- story movement, facilities, keys, thread, flags, and ending;
- compact inventory;
- active Drift expedition.

Every segment carries schema, generation, payload length, and checksum. After
all inactive-bank segments are written and read back byte-for-byte, a small
manifest atomically selects that generation. Loading falls back to the prior
bank if the active generation is corrupt. The complete redundant state uses
roughly 0.7 KB, below the 2 KB design gate and Pebble's 4 KB total.

The legacy 40-byte schema-1 through schema-3 record remains readable. A valid
0.1.x save is mapped into named campaign state and immediately committed as a
schema-4 bank without discarding resources, structures, elapsed timers, or
first-memory progress.

## Generated world

Terrain is regenerated from one saved 32-bit seed. Cell hashing and landmark
placement do not consume or mutate an RNG stream. Five distance bands contain
24 collision-free landmarks. The unpacked world is never saved; only the seed,
121-byte visibility mask, and three-byte landmark completion bitset are stored.
Entering a tile reveals it plus its orthogonal neighbors. Host tests pin a
golden world hash and landmark uniqueness for a fixed seed.

## Scene content

`content/scenes.json` is the authoring source. The compiler enforces stable
numeric scene IDs, known targets, reachable operations, loop-free graphs,
terminating paths, at most three choices, 20-character choice labels, and
80-character pages. It emits:

- a scene directory plus compact bytecode;
- a deduplicated string table;
- a size and longest-page report.

The watch reads directory records, one scene (maximum 45 bytes in 0.2.0), and
one string at a time through resource byte ranges. The 4 KB string pack is
never copied into RAM as a whole. VM opcodes cover text, choices, conditional
flags/resources, costs, rewards, trust, jumps, and results.

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

Holding SELECT for one second opens a clearly labeled testing menu. It covers
all resources, Fire, campaign movement, named guests, production assignments,
keys, thread, and direct preview of scenes 1–20. Movement advancement supplies
the prerequisite prototype rooms so testers can enter the relevant Drift band.
Every edit is clamped and persisted. Reset retains its separate confirmation.

## Timed house actions

Search Rooms, Feed Hearth, and Prepare Ration share a foreground-only two-second
action controller with 100 ms visual updates. Starting an action checks its
prerequisites through non-mutating functions owned by the rule layer. During
progress, SELECT, UP, DOWN, BACK, and long SELECT are ignored. Costs and rewards
are applied exactly once at completion and then persisted.

Elapsed-time simulation is deferred while a timed action is active, then caught
up normally afterward. This preserves the validity decision made at action
start without discarding Fire decay or guest production. The current outlined
bar and bottom-screen placement are deliberately provisional; future visual
tuning must not silently change the two-second duration, atomic completion, or
input-lock semantics.

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
