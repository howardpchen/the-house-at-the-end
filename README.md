# The House at the End

You wake in a small house floating in an empty, fragmented world. Keeping its
hearth alive makes the nearby rooms real. Lost travelers arrive, impossible
places reconnect beyond the front door, and every recovered memory changes
what the house can become.

The House at the End is an original, text-first survival and exploration game
for Pebble. It uses a gradual refuge-to-expedition progression while giving its
world, writing, systems, encounters, and implementation their own identity.

## Campaign foundation

Version 0.2.0 includes:

- native Pebble C support for Basalt and Emery;
- a button-driven house, workshop, guest, and expedition interface;
- two-second room searches, hearth feeds, and ration preparation, plus a hearth
  that loses one Fire every two minutes;
- Fire tiers that gate guest production, construction, crafting, guest
  assignments, the anchor line, and new expeditions;
- travelers who passively recover resources according to their assigned role;
- three unlockable house improvements;
- the original Crooked Hall expedition plus a deterministic 31×31 Drift with
  a 7×7 watch viewport and 24 stable landmark positions;
- a compiled, data-driven campaign path through five movements and three
  endings, with spoiler text kept out of this README;
- resource-backed scene pages and choices loaded through bounded buffers;
- a transactional, two-bank segmented save that migrates 0.1.x progress;
- deterministic host tests for the game-state rules;
- no phone, account, network connection, or background worker.

The spoiler-light product overview lives in
[`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md). Contributors can use the
**spoiler-heavy** [`docs/DESIGN_BIBLE.md`](docs/DESIGN_BIBLE.md) for the full
campaign, systems, technical budgets, and roadmap.

## Controls

- **UP / DOWN:** move through the current list.
- **SELECT:** enter a section or perform the highlighted action.
- **Hold SELECT for one second:** open the testing menu.
- **BACK:** return to the previous section; from the house, exit the app.

Searching a room, feeding the hearth, and preparing a ration each take two
seconds and show a progress bar. Buttons are held until the action completes,
then its cost and result are applied once. Elapsed-time catch-up is deferred
during the action so a valid action cannot become invalid halfway through.
The progress bar's current visual treatment is a prototype and will be tuned
separately from these timing and transaction rules.

Temporary story and action-result text uses the larger 24-pixel Gothic font and
remains visible for 4.2 seconds. Ordinary selected-action descriptions remain
compact.

The testing menu can change resources in steps of 10; adjust Fire, campaign
movement, named guests, keys, thread, and guest assignments; preview any
compiled scene; or reset through a separate confirmation screen. Test changes
save immediately.

The house status uses the larger 24-pixel Gothic font in a two-row grid. It
shows Fire, guests, `K` for kindling, `M` for remnants, `R` for rations, and
`C` for clarity. Selected-action descriptions spell out the full resource
names and costs.

Fire 0 is Cold; searching and feeding remain available. Fire 2 is Seen and
allows guests to arrive and work. Fire 3 is Held and enables ordinary
construction and ration preparation. Fire 5 is Shared and enables guest
assignment, the anchor line, and new expeditions. Fire 4 extends the Held
buffer without adding another unlock. Completed construction and active
expeditions are never erased when Fire falls.

## Build and test

The project requires Pebble SDK 4.x.

```sh
./scripts/test-game-state.sh
pebble build
```

The resulting universal PBW contains Basalt and Emery binaries at
`build/the-house-at-the-end.pbw`.

## License

The project is licensed under the Apache License 2.0. See [`LICENSE`](LICENSE).

The refuge-to-expedition pacing is inspired in part by *A Dark Room*. This is
an independent implementation and contains no upstream code, prose, artwork,
event data, or other game assets.
