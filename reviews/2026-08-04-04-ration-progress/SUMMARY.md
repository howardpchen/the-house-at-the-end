# Summary

Version 0.1.8 adds Prepare Ration to the shared two-second timed-action
controller. It validates the Held Fire, worktable, kindling, and remnant
requirements immediately; valid actions show `Preparing ration...`, block all
buttons, then spend one kindling and one remnant and add one ration at
completion.

The requested code review found and corrected two scaling risks in the 0.1.7
controller:

- UI preflight logic duplicated core Feed Hearth rules. Non-mutating
  `house_check_*` functions now own Feed Hearth and Prepare Ration validation,
  and the mutation functions reuse those checks.
- Completion treated every non-search timed action as Feed Hearth. Completion
  now uses an explicit switch over Search Rooms, Feed Hearth, Prepare Ration,
  and the invalid `NONE` state.

Elapsed-time catch-up is deferred during the input lock, then applied normally,
so a valid action cannot become invalid at a Fire-decay boundary and no elapsed
time is discarded. The outlined bar is documented as provisional while the
two-second duration, immediate validation, atomic completion, and input lock
are deliberate interaction rules.

Host tests, universal build, and Emery/Basalt runtime checks passed. The Time 2
CloudPebble install reported `App install succeeded`; physical timing feel
remains for the wearer to validate.
