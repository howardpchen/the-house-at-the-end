# Game Design Overview

This is the spoiler-light product and systems overview. The contributor-facing
[`DESIGN_BIBLE.md`](DESIGN_BIBLE.md) contains the complete plot, character
identities, endings, technical budgets, and roadmap.

## Design intent

The House at the End is a quiet survival and exploration game about making an
impossible place habitable, then leaving its safety to understand the
fragmented world around it. It is designed for short Pebble sessions without
requiring constant attention.

The project takes inspiration from the broad refuge-to-expedition escalation of
minimalist incremental adventures. Its implementation, world, language,
characters, encounters, progression, and narrative are original.

## Product pillars

- **Small actions, long consequences:** brief watch interactions matter across
  days.
- **A refuge worth protecting:** named guests and changing rooms make the house
  more than an economy screen.
- **Exploration as uncertainty:** expeditions trade safety for discoveries and
  resources.
- **Mystery through details:** objects, locations, and contradictions reveal
  the world gradually.
- **Pebble-native restraint:** four buttons, short text, local persistence, and
  timestamp simulation define the experience.

## Core loop

1. Search unstable rooms for kindling and remnants.
2. Feed the hearth to stabilize more of the house.
3. Welcome guests drawn to the light.
4. Assign guests to gathering, listening, and later specialist roles.
5. Build rooms and tools that expand the available decisions.
6. Turn resources into supplies and equipment.
7. Cross fragmented places, manage supplies, and resolve encounters.
8. Bring discoveries home, changing both the house and its inhabitants.

The house phase is dependable and increasingly productive. Expeditions are
brief, uncertain, and expensive. Neither phase should make the other obsolete.

## Primary resources

- **Kindling:** hearth fuel and common construction material.
- **Remnants:** matter recovered from unstable rooms and expeditions.
- **Rations:** compact expedition supplies.
- **Clarity:** the ability to keep a fragmented place mutually consistent.
- **Resolve:** expedition-only endurance during encounters.

Later progression adds a small number of specialized resources without turning
the interface into an inventory spreadsheet.

## Refuge progression

The hearth reveals the house gradually and attracts the first guest. Fire
fizzles by one point per minute, including elapsed time while the app is closed,
so kindling remains useful after the initial unlocks. Early construction opens
a guest room, worktable, and anchor line. Later facilities improve production,
mapping, equipment, recovery, and the Chronicle.

Guests are named characters, not anonymous population. Assignments describe
their current contribution and can change at the house. Elapsed production is
computed from saved timestamps when the app resumes, avoiding a permanent
background worker.

## Expedition loop

The player loads clarity, rations, resolve, and limited equipment before
leaving. Movement consumes supplies. A compact local viewport reveals routes,
hazards, events, and major landmarks. The player can press onward, resolve an
encounter, establish a safer route, or retreat with part of the cargo.

Ordinary cargo is at risk. Previously committed story discoveries and the
house itself are not erased by a failed expedition.

The planned complete world is a deterministic 31×31 Drift generated from a
saved seed. Visibility and cleared landmarks fit in compact bitsets. Both
Basalt and Emery show the same gameplay radius.

## Encounters

Encounters are short and turn-based. Typical actions preserve a concrete
detail, brace against an effect, listen for an alternate resolution, use
equipment, or retreat. Cooldowns are measured in turns rather than desktop-style
real-time timers.

At least half of expedition scenes should be navigation, observation, repair,
dialogue, or tradeoffs rather than combat.

## Controls and screens

- UP/DOWN move through lists and choices.
- SELECT opens or confirms.
- BACK returns one hierarchy level and exits only from the house hub.

The progressive hub leads to the hearth, workshop, guests, front door,
Chronicle, and later facilities. Basalt's 144×168 display defines the minimum
layout. Emery uses its 200×228 display for larger spacing and an extra context
line, not exclusive information.

## Persistence and connectivity

- local, versioned, checksummed state;
- no phone, account, backend, or network connection;
- no background worker;
- deterministic timestamp catch-up with a cap;
- generated terrain rebuilt from a saved seed;
- full planned save target below 2 KB, split across Pebble's 256-byte
  persistence values.

## Current vertical slice

Version 0.1.3 implements:

- two-second room searches and a hearth that loses one Fire per minute;
- first guest arrival and two elapsed-time roles;
- three foundational structures;
- ration and clarity preparation;
- one supply-limited expedition and encounter;
- Chronicle completion;
- versioned, checksummed persistence;
- host-side deterministic rules tests;
- successful complete emulator progression on Basalt and Emery.

Physical Time Steel and Time 2 validation remains outstanding.

## Full-game direction

The complete campaign expands the house, introduces a small named cast, opens a
seeded world map, adds equipment and varied encounters, and develops the
mystery across five major movements toward a consequential final decision.

Detailed plot information is intentionally omitted here.
