# Acceptance checklist

- PASS: valid Prepare Ration starts a 2000 ms timed action.
- PASS: progress text reads `Preparing ration...`.
- PASS: all buttons and the Test Menu long press are blocked during progress.
- PASS: prerequisites are checked without mutating state.
- PASS: invalid attempts fail immediately.
- PASS: completion spends one kindling and one remnant exactly once.
- PASS: completion adds one ration and saves the result.
- PASS: elapsed-time catch-up is deferred, not discarded, during progress.
- PASS: explicit completion branches cover all timed-action states.
- PASS: Search Rooms and Feed Hearth retain their existing behavior.
- PASS: progress UI fits Emery at 200x228.
- PASS: progress UI fits Basalt at 144x168.
- PASS: save schema and existing progress remain compatible.
- PASS: host rules tests pass, including non-mutating action checks.
- PASS: Basalt and Emery compile into the universal PBW.
- PASS: CloudPebble reports `App install succeeded` on the linked Time 2.
- PASS: granular prototype decisions are documented in the Obsidian project.
- PENDING: wearer validation of physical timing feel.
