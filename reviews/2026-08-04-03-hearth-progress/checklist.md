# Acceptance checklist

- PASS: valid Feed Hearth starts a 2000 ms timed action.
- PASS: progress text reads `Feeding hearth...`.
- PASS: progress bar updates every 100 ms.
- PASS: SELECT, UP, DOWN, BACK, and long SELECT are ignored during progress.
- PASS: Fire and kindling do not change before completion.
- PASS: completion raises Fire once and spends exactly two kindling.
- PASS: completion state is saved once.
- PASS: no-resource and maximum-Fire attempts fail immediately.
- PASS: Search Rooms retains its existing timing and behavior.
- PASS: progress UI fits Emery at 200x228.
- PASS: progress UI fits Basalt at 144x168.
- PASS: save schema and existing progress remain compatible.
- PASS: host game-state tests pass.
- PASS: Basalt and Emery compile into the universal PBW.
- PASS: CloudPebble reports `App install succeeded` on the linked Time 2.
- PENDING: wearer validation of physical timing feel.
