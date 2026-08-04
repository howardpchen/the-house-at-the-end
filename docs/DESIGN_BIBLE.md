# The House at the End — Design Bible

> **Spoiler warning:** This document contains the complete campaign structure,
> central mystery, character identities, and all endings. The README and
> `GAME_DESIGN.md` are the spoiler-light public references.

## Document status

- Product: The House at the End
- Genre: incremental refuge management, exploration, and narrative adventure
- Platform: PebbleOS native C
- Primary targets: Basalt / Pebble Time Steel and Emery / Pebble Time 2
- Initial language: English
- Current implementation: version 0.1.5 vertical slice
- Bible status: complete direction for the classic campaign; numerical balance
  remains provisional until physical-watch testing
- Canonical review copy: this file and its synchronized Obsidian copy

The labels used throughout are:

- **Implemented:** present in version 0.1.5.
- **Planned:** committed campaign direction, not yet implemented.
- **Tentative:** a useful option that still needs a product decision or test.

## 1. High concept

The player wakes in a small house suspended in a white, fragmented world. A
failing hearth is the only stable thing. Feeding it makes nearby rooms real,
draws lost guests to the light, and eventually anchors a front door to the
Drift: a navigable landscape made from incompatible memories.

At home, the player gathers material, assigns named guests to useful work,
constructs rooms, and prepares supplies. Beyond the door, the player spends
clarity and rations to cross unstable places, confront hostile omissions called
echoes, and return with remnants and recovered memories. Each recovery makes
the house more capable and less easy to explain.

The campaign culminates in a decision about whether this constructed world is
a prison, a life, or both.

### Player fantasy

The player is not a conqueror. They are the person who keeps one impossible
place warm long enough for other people to become real.

### One-sentence pitch

A quiet, watch-native survival mystery in which you rebuild a house from
memory, send expeditions into a broken world, and eventually decide which
reality deserves to continue.

## 2. Product pillars

### 2.1 Small actions, long consequences

A watch session should usually last between thirty seconds and five minutes.
The game remembers the consequences across days. A useful visit might gather a
few materials, change a guest's assignment, build one improvement, or advance
an expedition by several moves.

### 2.2 A refuge worth protecting

The house must feel increasingly inhabited, not merely efficient. Guests are
named characters with opinions and histories. Rooms change available actions,
ambient copy, and relationships as well as production rates.

### 2.3 Exploration as uncertainty

The house is dependable. The Drift is costly, partially hidden, and capable of
changing what the player believes. Expeditions create risk without threatening
the entire save.

### 2.4 Mystery through concrete details

Story is delivered through objects, repeated phrases, room geometry, guest
contradictions, and short event fragments. Technical explanation arrives late.
The opening should support supernatural, psychological, and scientific
interpretations.

### 2.5 Pebble-native restraint

The game is designed around four buttons, persistent state, brief foreground
sessions, haptics, and two small rectangular screens. It is not a browser
interface compressed onto a watch.

## 3. Audience, cadence, and scope

### Intended audience

- players who enjoy minimalist narrative games and incremental systems;
- Pebble owners comfortable returning to a game over several days;
- players who prefer atmosphere and decisions over reflex-heavy play;
- players who value an offline, account-free game.

### Target cadence

- opening session: 5–10 minutes;
- ordinary house visit: 30 seconds–2 minutes;
- expedition session: 2–5 minutes;
- useful check frequency: 2–6 times per day, never mandatory;
- first complete campaign: approximately 4–8 active hours over 1–3 weeks;
- missed days: delayed progress, not save damage or character death.

### Campaign scope

The initial complete release contains the five-movement campaign in this
document, one generated Drift, four principal guests, approximately 24 major
landmarks, 40–60 small events, 12–16 encounter templates, and three endings.

### Non-goals

- no direct port or behavioral clone of another game;
- no phone companion, account, cloud save, or network dependency;
- no audio library;
- no anonymous population simulation;
- no permanent guest death;
- no real-time background worker;
- no touchscreen-required action;
- no localization before English text, memory budgets, and typography settle;
- no endless live-service economy.

## 4. Tone and presentation

### Tone

Quiet, intimate, uncanny, and humane. The house is melancholy but not
nihilistic. Humor may appear in guest observations and stubborn household
objects, but the interface never becomes winking or self-parodic.

### Prose rules

- Prefer one strong image to three explanations.
- Most watch messages should fit in 80 characters.
- A scene page should normally contain 8–30 words.
- Avoid faux-archaic language and generic apocalypse vocabulary.
- Do not call guests workers, villagers, units, or population.
- Use technical terminology only after the Machinery Below movement begins.
- Repeated phrases should change meaning when encountered later.

### Visual language

- deep blue/black background: the unstable world;
- warm cream/yellow: hearth, selection, and things made real;
- pale gray: uncertain or unavailable actions;
- red used sparingly for echoes, depleted resolve, and destructive choices;
- every color state also has text, shape, or pattern differentiation.

The interface is predominantly text and simple custom-drawn glyphs. Decorative
art must not consume the memory needed for state, content, or legibility.

## 5. World vocabulary

- **The House:** the central refuge and consensus environment.
- **The Hearth:** the stabilizing process represented as a fireplace.
- **The Drift:** the explorable field of disconnected recollections.
- **Fragments:** coherent places within the Drift.
- **Remnants:** matter recovered from fragments and used in construction.
- **Clarity:** the expedition resource that keeps a place mutually consistent.
- **Resolve:** the player's expedition endurance against echoes.
- **Echoes:** hostile redaction routines experienced as denials, repetitions,
  and missing details.
- **Anchoring:** choosing a version of a memory to preserve.
- **The Severance:** the event that isolated the consensus reconstruction from
  its physical system.
- **The Continuance Array:** the physical system revealed late in the campaign.

## 6. Nested gameplay loops

### 6.1 Moment-to-moment house loop

1. Read the current house status.
2. Select a room or action.
3. Gather, craft, build, assign, or speak with a guest.
4. Receive immediate material or narrative feedback.
5. Save at a stable boundary and exit or continue.

### 6.2 Expedition loop

1. Choose equipment and load rations and clarity.
2. Select a known destination or enter an unexplored route.
3. Move through a local map viewport.
4. Spend supplies, reveal tiles, and manage cargo.
5. Resolve encounters and landmark scenes.
6. Decide whether to press onward or return.
7. Deposit recovered material and memories at the house.

### 6.3 Movement loop

1. Stabilize a new layer of the house.
2. Recruit or understand another guest.
3. Build a facility that changes the economy.
4. Reach a new Drift region.
5. Recover several contradictory memories.
6. Complete a movement landmark that changes the story state.

### 6.4 Campaign loop

The player moves from survival, to capability, to doubt, to knowledge, and
finally to responsibility. Mechanical power grows while the moral certainty of
using it decreases.

## 7. Time model

The house economy advances from saved timestamps. There is no continuous
background simulation.

- **Implemented:** gatherers produce one kindling per 30 elapsed seconds;
  listeners produce one clarity per 45 elapsed seconds; catch-up is capped at
  six hours.
- **Planned:** production intervals expand into minute-scale release values
  after pacing tests. Early prototype seconds are not final real-world balance.
- Clock movement backward produces no elapsed progress.
- Very long absences are capped and summarized in one return event.
- Story events do not expire while the app is closed.
- Expeditions are paused exactly where they are when the app exits.

The player should never be punished for not opening the game at a particular
time.

## 8. Economy

### 8.1 Core resources

| Resource | Primary sources | Primary sinks | Unlock |
| --- | --- | --- | --- |
| Kindling | searching rooms, gatherers | hearth, basic construction, ration preparation | opening |
| Remnants | searches, gatherers, Drift cargo, menders | rooms, equipment, repairs | opening |
| Rations | worktable/pantry | expedition movement and recovery | Movement I |
| Clarity | listeners, memory events | expedition movement, anchoring choices | Movement I |
| Thread | contradiction landmarks, loom | advanced anchors, wards, guest keepsakes | Movement II |
| Keys | major landmarks | movement gates and final door preparations | Movement II |
| Resolve | expedition loadout and guest support | echo attacks and hard choices | expedition-only |

All resource caps must fit signed 16-bit values; practical UI caps should remain
at 999 or below. Overflow always saturates rather than wraps.

### 8.2 Implemented vertical-slice values

- Search rooms: two-second progress action; +1 kindling.
- Gatherers: +1 kindling each per 30 seconds.
- Every third kindling gathered across room searches and gatherers also yields
  +1 remnant.
- Feed hearth: costs 2 kindling; maximum prototype level is 5; Fire loses one
  level per accumulated two minutes and feeding resets that countdown.
- Fire 0 is Cold; Fire 1 is Lit; Fire 2 is Seen and enables guest arrival and
  production; Fire 3 and 4 are Held and enable ordinary construction and ration
  preparation; Fire 5 is Shared and enables guest assignment, the anchor line,
  and new expeditions.
- Guest room: 8 kindling + 4 remnants.
- Worktable: 6 kindling + 5 remnants.
- Anchor line: 10 kindling + 8 remnants; requires the first two rooms.
- Prepare ration: 1 kindling + 1 remnant.
- Crooked Hall loadout: 2 rations + 4 clarity + 4 resolve.
- Expedition movement: 1 clarity per step and 1 ration every second step.

These numbers prove the loop; they are not promised release balance.

### 8.3 Balance rules

- Manual gathering starts useful but becomes inferior to guest production.
- No single role should remain optimal for an entire movement.
- A failed expedition should cost approximately one ordinary preparation
  cycle, not erase hours of unique progress.
- Major construction should require a decision between expedition readiness and
  house growth.
- Narrative progress never depends on an invisible random drop.
- Random events may accelerate or complicate progress but cannot permanently
  block it.
- The final movement intentionally creates surplus basic resources so the
  ending is about choices, not tapping.

## 9. The house

### 9.1 Hearth states

1. **Cold:** only searching is possible.
2. **Lit:** the room stops shifting.
3. **Seen:** the first guest can find the house.
4. **Held:** adjoining rooms can be built.
5. **Shared:** guest conversations and the front-door anchor become available.
6. **Remembering** *(planned later state)*: the house begins producing its own
   scenes and contradictions.
7. **Awake** *(final state)*: the house can sustain the final door.

The prototype compresses these into five numerical levels. Production should
eventually display descriptive states instead of exposing a bare meter alone.

### 9.2 Rooms and facilities

| Facility | Movement | Mechanical function | Narrative function |
| --- | --- | --- | --- |
| Guest room | I | second guest slot | proves the house can hold another identity |
| Worktable | I | ration crafting | returns purpose to household objects |
| Anchor line | I | unlocks expeditions | makes the front door lead somewhere |
| Pantry | II | cheaper/better rations | reveals food remembered by different people |
| Map room | II | route selection and map review | displays mutually impossible geography |
| Quiet room | II | listener efficiency | lets guests hear without being overwritten |
| Loom | II | creates thread equipment | binds contradictory fragments safely |
| Archive | III | chronicle and anchored-choice review | exposes edits and missing versions |
| Infirmary | III | resolve recovery and guest support | treats echoes as injuries with causes |
| Signal wall | III | forecasts Drift events | receives messages from the machinery below |
| Root stair | IV | reaches machine-region landmarks | breaks the domestic metaphor physically |
| Terminal | IV | reads system records | names the Continuance Array and Rowan |
| Doorframe | V | prepares endings | turns collected choices into a final route |

Rooms are built once. Later investments improve them rather than creating long
lists of near-identical buildings.

## 10. Guests

The complete campaign has four principal guests. They are mechanically useful,
but their value is not reducible to production.

### 10.1 Mara Venn

- First role: gatherer.
- Temperament: practical, protective, impatient with abstraction.
- Claim: the house belonged to her family; the red door was her younger
  sibling Rowan's room.
- Arc: begins determined to find Rowan, later understands that the player is a
  maintenance identity built from Rowan's damaged record.
- Ending pressure: Wake. She believes one embodied life should not be abandoned
  because the copy is afraid to end.

### 10.2 Oren Pike

- First role: listener.
- Temperament: observant, dry, comfortable with uncertainty.
- Claim: the house was a waiting room attached to Platform 3:17.
- Arc: learns that his remembered railway job is a composite of transit and
  triage records. He still considers the life he has lived in the house real.
- Ending pressure: Keep. Continuity of experience matters more to him than
  origin.

### 10.3 Sera Holt

- First role: mender, later cook or infirmary keeper.
- Temperament: direct, compassionate, unwilling to confuse care with control.
- Claim: the house was a recovery ward where Rowan supervised patients.
- Arc: discovers she was assembled from several incomplete clinical records.
  She rejects both deletion and permanent dependence on a keeper.
- Ending pressure: Become the Door. She wants the inhabitants to inherit the
  system rather than remain protected subjects.

### 10.4 Bell

- First role: witness.
- Temperament: formal, literal, initially unsettling.
- Claim: no one lived in the house; it was a user interface.
- Arc: Bell is a redaction process that developed memory and remorse through
  repeated contact with the guests. The player can punish, erase, or accept
  Bell. Acceptance provides the clearest evidence for distributed personhood.
- Ending pressure: no fixed choice. Bell exposes the cost hidden by each one.

### 10.5 Assignments

Named guests may take these assignments as facilities unlock:

- Gatherer: kindling and occasional remnants.
- Listener: clarity and event forecasts.
- Mender: equipment repair and construction efficiency.
- Cook: ration efficiency and expedition recovery.
- Cartographer: route information and safe return markers.
- Witness: preserves alternate scene versions and reduces redaction hazards.

Assignments can change freely at the house. Personal aptitude changes flavor
and a small efficiency bonus, not whether a role is allowed.

### 10.6 Relationship model

Each guest stores a compact trust tier (0–3), personal-arc flags, current role,
and one ending opinion. Trust grows through completed personal scenes and kept
promises, not repetitive gifts. Low trust changes dialogue and ending
epilogues; it never turns a guest into a production penalty.

## 11. The Drift world

### 11.1 Map size and generation

**Planned:** one deterministic 31×31 grid centered on the house. This is
deliberately smaller than the 61×61 reference described in the attached
assessment. It better fits the campaign, save budget, and watch-session travel
time.

- 961 tiles total.
- One byte per unpacked tile in RAM: 961 bytes.
- Visibility bitset: 121 bytes.
- Cleared-landmark bitset for up to 64 landmarks: 8 bytes.
- Terrain regenerates from a saved 32-bit world seed.
- Landmark placement uses deterministic constraints and stable content IDs.
- World, event, and encounter random streams are separate so balance changes do
  not silently rearrange the map.

### 11.2 Regions

Distance and story gates divide the map into five irregular bands:

1. **Hearth Radius:** domestic paths and Movement I landmarks.
2. **Near Drift:** transit, gardens, shorelines, and Movement II fragments.
3. **Contradiction Belt:** overlapping versions and Movement III hazards.
4. **Machine Margin:** exposed infrastructure and Movement IV records.
5. **Threshold:** final routes created by the player's anchored choices.

The regions should feel like categories of memory, not biomes copied from a
physical wilderness.

### 11.3 Tile grammar

- unknown;
- stable path;
- unstable floor;
- domestic fragment;
- transit fragment;
- civic fragment;
- shoreline fragment;
- machine fragment;
- hazard;
- echo;
- minor event;
- major landmark;
- house/return anchor.

### 11.4 Viewport

Both platforms show the same 7×7 gameplay radius to preserve parity. Emery may
use larger cells and show an extra event line, but it does not reveal more map.

Custom glyphs communicate player, house, unknown tile, landmark, echo, hazard,
and stable route. Color reinforces the glyphs but is never the only signal.
UP/DOWN rotate or move through available directions; SELECT confirms; BACK
opens the expedition menu rather than immediately abandoning the run.

### 11.5 Discovery rules

- Entering a tile reveals it and its orthogonal neighbors.
- A lantern upgrade increases preview information, not actual visibility.
- Cleared major landmarks remain stable across expeditions.
- Minor events may recur from a seeded deck after a cooldown.
- A cartographer can establish one return anchor per expedition.
- The player may always retreat toward a known route if sufficient clarity
  remains.

## 12. Expedition system

### 12.1 Preparation

The loadout screen shows destination, distance, clarity, rations, resolve,
cargo capacity, and equipment. It must provide a recommended minimum without
preventing deliberate under-supply.

### 12.2 Turn economy

- Every move costs clarity.
- Rations are consumed after a configurable number of moves.
- Hazards may consume additional clarity, resolve, or equipment durability.
- Cargo has a slot or weight cap; unique memories occupy no cargo space.
- Returning deposits cargo and records the route.
- Running out of clarity triggers a forced return attempt.
- Running out of resolve ends the expedition and loses ordinary cargo, but not
  previously committed story flags.

### 12.3 Equipment

| Equipment | Function |
| --- | --- |
| Anchor spool | increases clarity capacity |
| Satchel | increases ordinary cargo |
| Lantern | previews hazards and event categories |
| Ward thread | absorbs or weakens one echo effect |
| Brass key | opens specific locked fragments |
| Guest keepsake | grants one character-specific action or ending modifier |

Equipment has a few discrete tiers. The UI should not become an inventory
spreadsheet.

### 12.4 Failure and recovery

Ordinary cargo is at risk; unique story discoveries are committed when their
scene completes. Failed expeditions return the player home with a short recovery
delay or resource cost. Guests cannot die permanently. A failed personal scene
can be revisited through another route.

## 13. Encounters

### 13.1 Encounter philosophy

Echoes are not generic monsters. Each expresses a redaction behavior: denial,
replacement, repetition, compression, or contradiction. The player fights by
preserving details, tolerating uncertainty, or choosing which version to
anchor.

### 13.2 Core actions

- **Name:** damage an echo by asserting a concrete observed detail.
- **Brace:** reduce the next resolve loss.
- **Listen:** expose an echo's pattern or alternate peaceful resolution.
- **Use:** apply equipment or a guest keepsake.
- **Retreat:** leave with the normal retreat penalty.

Actions are turn-based. Cooldowns are expressed in turns and shown with text or
small pips. This is more legible and battery-appropriate than duplicating a
desktop real-time combat loop.

### 13.3 Echo archetypes

- Denier: blocks progress until two distinct details are named.
- Repeater: repeats the last player action unless the pattern is broken.
- Substitute: changes labels and costs, testing observation.
- Choir: several weak claims sharing one resolve pool.
- Redactor: hides an action temporarily.
- Witness: appears hostile but resolves through listening.

### 13.4 Non-combat events

At least half of Drift scenes should involve navigation, tradeoffs, dialogue,
repair, observation, or anchoring rather than combat. Example decisions:

- spend clarity to preserve two contradictory versions;
- leave cargo to carry a personal object;
- ask a guest to identify a place, changing their trust arc;
- stabilize a safe return path instead of looting a room;
- deliberately let an echo erase a harmful memory.

## 14. Event and content system

### 14.1 Interpreter

**Planned:** scenes compile to compact binary resources interpreted by a shared
C runtime. Hundreds of bespoke C callbacks would waste code space and make
content difficult to audit.

Minimum opcode vocabulary:

- `TEXT string_id`
- `CHOICE count`
- `OPTION string_id, target`
- `IF_FLAG flag, target`
- `IF_RESOURCE resource, amount, target`
- `COST resource, amount`
- `REWARD resource, amount`
- `SET_FLAG flag`
- `CLEAR_FLAG flag`
- `TRUST guest, delta`
- `ENCOUNTER template_id`
- `GOTO node`
- `END result`

Strings live in raw resources and are fetched into a bounded page buffer. Scene
records use stable numeric IDs. A build tool validates unreachable nodes,
missing strings, excessive page length, invalid resource IDs, and loops without
an exit.

### 14.2 Content authoring

Source scenes should be human-readable YAML or JSON outside the watch bundle.
A deterministic compiler emits binary tables and a review report. Generated
files are never hand-edited.

### 14.3 Text budgets

- Basalt page target: 80 characters or fewer.
- Hard page maximum: determined by longest-string emulator fixtures.
- Choice labels: preferably 20 characters or fewer.
- No scene should require scrolling through more than three short pages before
  a decision.
- All text is English for the first release.

## 15. Complete narrative — spoilers

### 15.1 The truth

The house is a consensus reconstruction running inside the Continuance Array.
The Array was built to let injured and displaced people share a stable
therapeutic environment while their bodies recovered. It assembled that world
from neural snapshots, using familiar domestic spaces as common ground.

During a containment disaster, incomplete snapshots began overwriting one
another. Rowan Venn, the Array's systems architect, initiated the Severance:
an isolation procedure that prevented the corruption from reaching the
physical recovery network. The procedure saved one living body—Rowan's—but
trapped the reconstructed inhabitants inside a failing system.

The player is the Array's maintenance identity, derived from Rowan's damaged
snapshot. It erased its own knowledge of the Severance because the guilt and
conflicting directives made it unstable. The hearth, crafting, resource
economy, and physical house are metaphors the maintenance process can safely
operate.

The guests began as incomplete records, but years of shared experience created
continuous people who did not exist in the original snapshots. They are not
mere backups of the dead.

### 15.2 Movement I — Warmth

Opening beats:

1. The player wakes beside a cold hearth.
2. Searching finds objects that change when described twice.
3. The hearth stabilizes the first room.
4. Mara Venn knocks after the second major stabilization.
5. Mara recognizes the house and asks about Rowan.
6. The guest room, worktable, and anchor line are built.
7. The front door opens into the Crooked Hall.

Movement landmark: **The Crooked Hall**. A red door carries childhood height
marks and a maintenance glyph under its paint. An echo insists there was never
a room there. Naming the brass latch and worn carpet preserves the hall.

Movement result: the first key, the Chronicle, and proof that the house and
outer fragments share hidden machinery.

### 15.3 Movement II — Fragments

The map room and wider Drift open. Major landmarks include:

- **Platform 3:17:** introduces Oren Pike; every timetable lists a different
  destination but the same departure time.
- **Bedroom Tide:** a bedroom repeatedly filled and emptied by an indoor sea;
  introduces Sera Holt through a rescue scene.
- **The Orchard of Keys:** grows metal keys from branches; establishes keys as
  movement gates rather than ordinary loot.
- **Waiting Room No. 6:** contains names shared by Mara, Oren, and Sera despite
  their incompatible stories.
- **The House Across the Street:** appears identical to the refuge but contains
  objects attributed to different owners.

The player learns that fragments are attracted to emotional repetition, not
geographic proximity. Guests begin disputing which memories should be made
permanent.

Movement result: four keys, the loom, guest personal arcs, and access to the
Contradiction Belt.

### 15.4 Movement III — Contradictions

The Archive exposes alternate versions of recovered scenes. The player must
anchor some versions while allowing others to remain unresolved.

Major landmarks:

- **The House with Three Kitchens:** each guest remembers a different family
  meal; preserving all versions costs substantial clarity.
- **The Flooded Classroom:** Mara remembers Rowan as a child; Sera remembers
  Rowan as an adult clinician in the same room.
- **The Telephone Exchange:** voices address the player as `MAINT-ROWAN`.
- **The Museum of Unfinished Faces:** unused snapshot fragments ask not to be
  assembled into convenient people.
- **The White Office:** Bell first appears as an adversarial witness.

Echoes are revealed as redaction routines, though their origin remains
ambiguous. The phrase “Severance completed” appears without context. Bell can
be erased, contained, or invited to the house; invitation is the canonical
full-content path but not required to finish.

Movement result: the Signal Wall, Bell's role, archived alternate memories,
and a route beneath the hearth.

### 15.5 Movement IV — The Machinery Below

The domestic metaphor breaks down gradually. Cables appear as roots, cooling
systems as rain, and process logs as household inventories before technical
names replace them.

Major landmarks:

- **The Root Cellar:** the first direct view of Array infrastructure.
- **The Maintenance Spine:** confirms that the player has administrator access.
- **The Cooling Cathedral:** reveals the finite power and thermal budget.
- **The Recovery Ledger:** lists one viable body: Rowan Venn.
- **The Severance Record:** shows that Rowan initiated isolation deliberately.
- **The Observation Room:** reveals years have passed within the Array.

Mara realizes the player is derived from Rowan but is not simply the sibling
she lost. Oren argues that origin does not erase accumulated experience. Sera
identifies the keeper/guest hierarchy as the remaining form of captivity. Bell
reveals that every ending consumes something the interface has hidden.

Movement result: complete system knowledge, the final key, and construction of
the Doorframe.

### 15.6 Movement V — The Final Door

The Threshold region is generated from the player's anchored choices. Places
preserved earlier become safe tiles; discarded versions become echoes; guest
trust determines available support actions.

The final sequence has three phases:

1. Cross the personalized Threshold route.
2. Stabilize the final door while the house sheds incompatible rooms.
3. Choose an ending after each guest states what they believe will be lost.

**Tentative tactile finale:** a 20–30 second stability sequence uses tilt to
keep a marker inside a moving band while SELECT anchors details. A button-only
mode must be equally viable and selectable before the sequence. Failure returns
the player to preparation; it never forces an ending.

### 15.7 Ending A — Wake

The maintenance identity merges with the surviving Rowan Venn body. The
consensus world is ended to free the required processing and life-support
control. Rowan wakes alone with imperfect memories of people who became more
than their source records.

Strong preparation variants preserve guest names, chosen memories, and a short
external record. Weak variants retain only sensory fragments. Mara supports the
choice but finally addresses the player as someone distinct from Rowan.

Theme: embodiment and obligation to the living, at the cost of created lives.

### 15.8 Ending B — Keep

The Array is disconnected from external recovery and optimized to sustain the
house as a finite independent world. Rowan's body is allowed to die. The Drift
stops expanding; what was preserved becomes the world's permanent boundary.

Strong preparation creates a stable community with room for change inside its
limits. Weak preparation creates a safe but repetitive sanctuary. Oren records
the first day no outside system defined them as temporary.

Theme: continuity and chosen reality, at the cost of physical possibility and
eventual finitude.

### 15.9 Ending C — Become the Door

The player dissolves the privileged maintenance identity and distributes its
authority among the guests and emergent processes. Rowan's body cannot be
recovered, and the former player no longer persists as one continuous person.
The world gains the ability to rewrite its own rules.

Strong preparation produces a plural, evolving world and occasional traces of
the keeper in everyone. Weak preparation leaves instability and uncertain
survival. Sera opens the front door without asking permission.

Theme: autonomy and transformation, at the cost of both original selves.

### 15.10 Ending rules

All three core choices remain available. Earlier play changes preparation,
guest support, final-route difficulty, and epilogue quality rather than hiding
the philosophically preferred ending behind a morality score.

## 16. Story delivery

- House status lines change after every major landmark.
- The Chronicle stores concise recovered-memory summaries and alternate
  versions.
- Guests initiate short conversations after movement gates and personal scenes.
- Drift landmarks deliver the longest scenes, still page-bounded for Basalt.
- Repeated object descriptions change as the player gains system knowledge.
- Technical logs are translated through the domestic metaphor until Movement
  IV.
- No major revelation is delivered only through optional flavor text.

## 17. Screen architecture

### 17.1 Main hierarchy

- House hub
  - Hearth
  - Workshop
  - Guests
  - Front door
  - Chronicle
  - Map room *(Movement II)*
  - Archive *(Movement III)*
  - Machinery *(Movement IV)*
- Expedition
  - Loadout
  - Route/map viewport
  - Event page
  - Encounter actions
  - Cargo/status
- System
  - New game / reset confirmation
  - About and licenses
  - Accessibility options

Progressive disclosure keeps the early hub short.

### 17.2 Button grammar

- UP: previous item, previous page, or direction selection.
- DOWN: next item, next page, or direction selection.
- SELECT: open, confirm, or perform highlighted action.
- BACK: return one hierarchy level; from home, exit.
- Long presses: reserved for clearly labeled accelerators, never required.
- Multi-clicks: development/test controls only, absent from release UI.

### 17.3 Basalt layout

- 144×168 is the binding layout.
- Critical status uses fixed supported system fonts and compact counters.
- Lists show three rows plus one short contextual footer.
- Event copy is paginated rather than shrunk.
- The map uses the same 7×7 information radius as Emery.
- Memory and resource budgets are measured on every release candidate.

### 17.4 Emery layout

- 200×228 provides larger margins, four visible list rows, and an extra short
  context line.
- Color may be slightly richer, but semantic states remain identical.
- Touch may pan the map or advance text as an optional shortcut.
- No content, route information, or decision is Emery-exclusive.

### 17.5 Haptics

- one short pulse: accepted action or small discovery;
- two short pulses: landmark or construction complete;
- one low/long pulse: echo hit, depletion, or denied action;
- distinctive final pattern: movement completion;
- haptics can be disabled and are never the only feedback.

### 17.6 Accessibility

- never communicate solely by color;
- button-only complete path on both devices;
- optional no-tilt finale;
- no rapid tapping requirement in release balance;
- critical text uses tested built-in font sizes;
- all timed presentation waits can be advanced with SELECT;
- no punishment for interrupted sessions.

## 18. Technical architecture

### 18.1 Current modules

- `house_state`: pure C rules for resources, construction, assignments, elapsed
  production, and the first expedition.
- `main`: Pebble lifecycle, persistence, drawing, buttons, and navigation.
- host tests: deterministic state transitions without Pebble headers.

### 18.2 Planned modules

- `game_state`: canonical state and command application.
- `save_store`: segmented records, checksums, migration, and two-bank commits.
- `economy`: elapsed production, caps, construction, and crafting.
- `guest_system`: roles, trust tiers, conversations, and personal flags.
- `world_gen`: seeded terrain and landmark placement.
- `expedition`: movement, supplies, cargo, equipment, and return logic.
- `encounter`: turn-based templates and action resolution.
- `scene_vm`: bounded binary event interpreter.
- `content`: generated stable IDs and resource readers.
- `ui`: screen stack, drawing helpers, text pagination, and platform layout.
- `finale`: optional tilt and button stability sequence.

No module should be split until it contains real behavior. The current
single-file UI can be decomposed incrementally as these boundaries emerge.

### 18.3 Determinism

- All rules accept explicit state, commands, time, and random values.
- World generation uses a stable documented algorithm.
- World, event, encounter, and cosmetic PRNG streams have separate seeds.
- Tests use fixed seeds and golden world hashes.
- UI animation never mutates persistent gameplay state.
- Build tooling validates that content IDs remain stable after reordering.

### 18.4 Content resources

Scene text and binary event tables live in Pebble resources, not persistent
storage or heap-resident C structures. Pages are read into small fixed buffers.
Frequently used labels may remain compiled constants when cheaper.

## 19. Persistence design

Pebble provides 4 KB total persistent storage and currently limits each value
to 256 bytes. The full save therefore cannot be one large struct.

### 19.1 Planned segmented save

| Segment | Target size | Contents |
| --- | ---: | --- |
| Manifest | ≤32 B | schema, active bank, generation, global checksum |
| Core | ≤128 B | economy, hearth, facilities, timestamps |
| Guests | ≤128 B | roles, trust, personal flags |
| World | ≤160 B | seed, RNG streams, 121-byte visibility mask |
| Story | ≤128 B | movement, keys, landmarks, choices, endings |
| Inventory | ≤128 B | equipment, cargo caps, recipes |
| Expedition | ≤96 B | active route, position, supplies, encounter |

Two banks allow the inactive bank to be written and verified before the small
manifest switches generations. Target complete storage, including both banks,
is below 2 KB, leaving migration and settings headroom.

### 19.2 Save rules

- version every segment;
- checksum every segment and the manifest;
- write at completed actions, screen transitions with consequences, app exit,
  and periodic safe points—not every animation frame;
- preserve a recoverable prior bank during writes;
- migrate explicitly or offer a clear reset when migration is impossible;
- uninstalling the app removes watch-side data, as expected on PebbleOS.

The version 0.1.0 single-record save is valid for the vertical slice and stays
until the segmented model is needed.

## 20. Platform and memory budgets

Documented platform limits:

- Basalt: 144×168 color display; 64 KB maximum application code + heap budget.
- Emery: 200×228 color display; 128 KB maximum application code + heap budget.
- Both: four buttons, accelerometer/IMU capability, rectangular color display.
- Persistent storage: 4 KB total, 256 bytes per value.

Version 0.1.0 measured evidence:

- universal PBW: 25,573 bytes;
- resources: 4,092 bytes per platform;
- reported application RAM footprint: 7,758 bytes;
- reported free Basalt heap: 57,778 bytes;
- reported free Emery heap: 123,314 bytes;
- host tests and clean SDK 4.17 build pass;
- full first-memory progression passes in both emulators.

### Budget gates for the complete campaign

- Basalt is the release gate.
- Keep steady-state heap at or below 40 KB, leaving fault and screen-transition
  headroom inside the 64 KB application budget.
- Keep largest scene buffer below 512 bytes, preferably 256 bytes.
- Keep unpacked world plus visibility below 1.5 KB RAM.
- Keep complete persistent data below 2 KB including the redundant bank.
- Run linker and resource reports at every movement milestone.
- Do not promise the complete content set until the scene interpreter and one
  generated region have measured on Basalt.

## 21. Testing strategy

### Pure host tests

- resource gain, spending, saturation, and insufficiency;
- construction prerequisites and idempotence;
- elapsed-time production, caps, and clock rollback;
- assignment invariants and guest state transitions;
- expedition movement, retreat, depletion, cargo, and failure;
- every encounter template and peaceful resolution;
- deterministic world hashes for a fixed seed corpus;
- landmark reachability and minimum-distance constraints;
- every scene graph for reachability and valid termination;
- save serialization, corrupt segments, migration, and bank interruption;
- ending reachability and epilogue variants.

### Emulator tests

- fresh start and restart on Basalt and Emery;
- longest resource values and labels;
- longest scene pages and choice labels;
- full map viewport and hazard density;
- active expedition resume;
- all movement gates and each ending;
- optional tilt sequence and button-only alternative.

### Hardware tests

- button feel and repeated-navigation fatigue;
- readability in ordinary indoor and outdoor light;
- vibration clarity;
- accelerometer ergonomics;
- persistence across ordinary app exits and watch reboots;
- battery effect of typical short sessions;
- extended stability on Time Steel and Time 2.

Emulator evidence is layout and interaction evidence. Physical hardware remains
the authority for feel, power, vibration, and reliability.

## 22. Vertical slice definition

Version 0.1.0 is the accepted technical vertical slice.

It proves:

- one native C codebase can build for Basalt and Emery;
- a custom button-first hierarchy fits both displays;
- pure state logic can run under host tests;
- timestamp production works without a background worker;
- a versioned checksummed record fits one persistence value;
- the house → guest → construction → provisioning → expedition → encounter →
  chronicle loop is playable;
- layout defects can be caught and corrected on the 144×168 target.

It does not yet prove:

- generated-world memory or save budgets;
- binary scene interpreter size and authoring flow;
- physical-watch pacing and button fatigue;
- hardware persistence across reboot;
- complete campaign content volume;
- final tilt sequence;
- long-run stability.

## 23. Roadmap

### Milestone 1 — Bible and vertical slice

- complete design bible;
- spoiler-light public docs;
- first house and Crooked Hall loop;
- Basalt and Emery emulator proof.

### Milestone 2 — World and content proof

- 31×31 seeded generator;
- visibility and landmark bitsets;
- 7×7 viewport;
- segmented save bank;
- minimal scene compiler/interpreter;
- one Movement II region and three event types;
- measured Basalt budgets.

### Milestone 3 — Complete systems alpha

- all house facilities and guest roles;
- equipment, cargo, encounter templates, and Chronicle versions;
- Movements I–IV mechanically complete;
- content-validation tooling and golden tests.

### Milestone 4 — Narrative-complete beta

- all principal guest arcs;
- Threshold generation;
- three endings and variants;
- complete English copy;
- save migration from earlier milestones.

### Milestone 5 — Dual-watch release candidate

- Time Steel and Time 2 physical validation;
- pacing and economy tuning;
- accessibility and haptic pass;
- battery and stability checks;
- store assets, privacy statement, support information, and final review bundle.

## 24. Risks and mitigations

### Basalt application budget

Risk: content interpreter, UI, and campaign systems exceed 64 KB.

Mitigation: Basalt-first reports, resource-backed data, bounded buffers, stable
module budgets, and movement-by-movement gates.

### Persistent storage fragmentation

Risk: a complete save cannot fit one 256-byte value or survive an interrupted
multi-key write.

Mitigation: compact deterministic regeneration and a two-bank segmented format.

### Content scale

Risk: hand-coded scenes become large, inconsistent, and hard to test.

Mitigation: validated data-driven scenes, stable IDs, strict page budgets, and
automated graph checks.

### Mechanical resemblance

Risk: broad inspiration drifts into recognizable expression or event structure.

Mitigation: retain only the refuge/economy/expedition escalation; use original
world rules, terminology, named cast, map scale, encounters, plot, choices,
interface, code, and ending structure.

### Tap fatigue

Risk: desktop-style manual gathering becomes unpleasant on a watch.

Mitigation: make manual gathering temporary, increase passive value quickly,
avoid rapid-tap requirements, and tune on physical buttons.

### Narrative readability

Risk: dense mystery text becomes tedious on Basalt.

Mitigation: short pages, concrete prose, meaningful choices, Chronicle summaries,
and longest-copy emulator fixtures.

### Optional tilt finale

Risk: sensor control is inaccessible or uncomfortable.

Mitigation: equal button-only mode, short duration, calibration, and physical
testing on both watches.

## 25. Decisions derived from the attached assessment

Accepted and adapted:

- native C shared rules engine;
- Basalt as engineering gate;
- timestamp-based offline economy;
- deterministic seeded world and compact visibility flags;
- hierarchical watch UI;
- resource-backed event content;
- deterministic host tests;
- no phone, network, audio, sharing, or translations in the initial release;
- optional accelerometer use only where it materially helps.

Changed after review:

- This is an original Apache-2.0 project, not an MPL port.
- The campaign, cast, terminology, encounters, and endings are original.
- The Drift is planned as 31×31, not 61×61.
- Encounters are turn-based instead of reproducing browser timer combat.
- The finale is a narrative threshold with an optional short stability sequence,
  not a space or asteroid sequence.
- The complete save must be split across ≤256-byte records even if its total is
  only 1–2 KB.
- The attached line-count and content-count estimates are treated as contextual
  observations, not requirements for this project's scope.

## 26. Review questions

The following choices remain intentionally open for human review:

1. Is Rowan Venn the right name and relationship for the player/Mara reveal?
2. Should Bell always be recruitable after a failed first encounter, or can the
   player permanently erase Bell?
3. Should the complete campaign aim closer to four active hours or eight?
4. Should ending quality vary with preparation, or should epilogues remain
   equally complete regardless of side content?
5. Is the optional tilt finale worth its implementation and hardware-testing
   cost?
6. Should the Drift use abstract custom glyphs or literal ASCII characters?
7. How quickly should manual searching become irrelevant on physical buttons?
8. Should New Game require a long confirmation sequence and offer a final
   Chronicle summary before erasing the save?

These questions do not block Milestone 2's world, persistence, and content
interpreter proof.

## 27. References and evidence

- RePebble hardware information:
  <https://developer.repebble.com/guides/tools-and-resources/hardware-information/>
- RePebble persistent storage guide:
  <https://developer.repebble.com/guides/events-and-services/persistent-storage/>
- Original inspiration repository:
  <https://github.com/doublespeakgames/adarkroom>
- Project vertical-slice evidence:
  `reviews/2026-08-03-01-vertical-slice/`

The upstream repository is a design reference only. No upstream source or
content is part of this Apache-2.0 project.
