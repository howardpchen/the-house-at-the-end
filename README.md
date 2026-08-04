# The House at the End

You wake in a small house floating in an empty, fragmented world. Keeping its
hearth alive makes the nearby rooms real. Lost travelers arrive, impossible
places reconnect beyond the front door, and every recovered memory changes
what the house can become.

The House at the End is an original, text-first survival and exploration game
for Pebble. It uses a gradual refuge-to-expedition progression while giving its
world, writing, systems, encounters, and implementation their own identity.

## First playable milestone

The current vertical slice includes:

- native Pebble C support for Basalt and Emery;
- a button-driven house, workshop, guest, and expedition interface;
- two-second room searches and a hearth that loses one Fire per minute;
- travelers who passively recover resources according to their assigned role;
- three unlockable house improvements;
- a supply-limited expedition and turn-based encounter;
- versioned, checksummed watch-side persistence with elapsed-time production;
- deterministic host tests for the game-state rules;
- no phone, account, network connection, or background worker.

The spoiler-light product overview lives in
[`docs/GAME_DESIGN.md`](docs/GAME_DESIGN.md). Contributors can use the
**spoiler-heavy** [`docs/DESIGN_BIBLE.md`](docs/DESIGN_BIBLE.md) for the full
campaign, systems, technical budgets, and roadmap.

## Controls

- **UP / DOWN:** move through the current list.
- **SELECT:** enter a section or perform the highlighted action.
- **BACK:** return to the previous section; from the house, exit the app.

Searching a room takes two seconds and shows a progress bar. Buttons are held
until the search completes, then its reward is granted once.

Temporary story and action-result text uses the larger 24-pixel Gothic font and
remains visible for 4.2 seconds. Ordinary selected-action descriptions remain
compact.

The compact status line uses `K` for kindling, `M` for remnants, `R` for
rations, and `C` for clarity. Selected-action descriptions spell out the full
resource names and costs.

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
