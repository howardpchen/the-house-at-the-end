# Game Design

## Design intent

The House at the End is a quiet survival game about making an impossible place
habitable, then leaving its safety to learn what it is. It should work in short
watch sessions without requiring constant attention. Progress comes from a
mixture of deliberate actions, elapsed time, allocation decisions, expeditions,
and discoveries.

The design borrows a broad incremental-adventure rhythm, not another game's
code, writing, terminology, event sequence, balance, map, or ending.

## Core loop

1. Search unstable rooms for kindling and remnants.
2. Feed the hearth to stabilize more of the house.
3. Welcome guests drawn to the light.
4. Assign guests to gather material or listen for memories.
5. Build rooms and tools that expand the available decisions.
6. Turn resources into rations and clarity for expeditions.
7. Cross fragments of remembered places, manage supplies, and face echoes.
8. Bring discoveries home, changing both the house and its inhabitants.

The house phase is dependable and increasingly productive. Expeditions are
brief, uncertain, and expensive. Neither phase should make the other obsolete.

## Resources

- **Kindling:** immediate fuel and common construction material.
- **Remnants:** physical scraps recovered from unstable rooms and memories.
- **Rations:** compact supplies prepared for expeditions.
- **Clarity:** the ability to hold a remembered place together while inside it.
- **Resolve:** expedition-only endurance against hostile echoes.

## Refuge progression

The hearth has five stability levels. Early levels reveal the house and attract
the first guest. Construction then opens three foundational improvements:

- **Guest room:** makes space for another traveler.
- **Worktable:** turns kindling and remnants into expedition rations.
- **Anchor line:** connects the front door to distant fragments and unlocks
  expeditions; it requires the first two improvements.

Guests can be assigned as gatherers or listeners. Gatherers recover kindling;
listeners recover clarity. Production is calculated from timestamps when the
app resumes, so the game does not consume Pebble's background-worker slot.
Catch-up is capped to prevent clock changes or long absences from overwhelming
the economy.

## Expedition loop

An expedition loads clarity and rations from the house. Every step consumes
clarity; every second step also consumes a ration. Resolve is lost during
hostile encounters. Reaching a destination deposits recovered remnants and a
memory automatically. Retreating returns only part of the carried material;
running out of a critical supply loses the carried material.

The first vertical-slice destination is **The Crooked Hall**, a domestic space
that cannot decide which house it belongs to. Its hostile echo is subdued by
naming details that the simulation tried to discard.

Later expeditions should introduce branching routes, equipment, choices with
persistent consequences, guest-specific reactions, and several kinds of
encounter beyond combat.

## Controls and screens

The app follows Pebble's conventional button grammar:

- UP and DOWN move through lists.
- SELECT enters the highlighted section or performs its action.
- BACK returns to the previous section and exits only from the main house view.

The initial screen is the house hub. It leads to the hearth, workshop, guests,
front door, and chronicle as those sections become relevant. The interface must
remain readable at 144x168 on Basalt and use Emery's larger display for more
breathing room rather than additional required information.

## Persistence and failure

The complete state is stored locally in a versioned, checksummed record. No
phone or network service is required. Failed expeditions cost loaded supplies
and carried discoveries, but they do not erase the house or its guests. The
game's tension comes from uncertain journeys, not irreversible save loss.

## Full campaign shape

The intended campaign has five movements:

1. **Warmth:** stabilize the house and meet the first guests.
2. **Fragments:** recover rooms and places that should not coexist.
3. **Contradictions:** learn that every guest remembers the house differently.
4. **The machinery below:** discover what assembled this consensus world.
5. **The final door:** choose whether to wake, preserve the inhabitants, or
   surrender the protagonist's identity so the constructed world can continue.

Each movement expands the economy and expedition vocabulary while keeping the
same short-session interaction model.
