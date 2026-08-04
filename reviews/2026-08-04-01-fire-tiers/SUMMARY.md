# Summary

Version 0.1.5 makes Fire a gameplay gate and slows its decay from one minute to
two minutes per point.

- Cold 0: searching and feeding remain available.
- Lit 1: the first room is stable, but higher-tier actions remain paused.
- Seen 2: the first guest can arrive and guests can produce resources.
- Held 3-4: ordinary construction and ration preparation are available.
- Shared 5: guest reassignment, anchor-line construction, and new expeditions
  are available.

Built structures are retained below their required tier. An expedition already
underway remains playable after Fire falls. Elapsed-time guest production is
credited only until Fire falls below Seen.

Host rules tests and universal Basalt/Emery builds passed. Emery emulator
installation and tier UI screenshots passed. The Time 2 CloudPebble install
reported `App install succeeded`; physical interaction feel remains for the
wearer to validate.
