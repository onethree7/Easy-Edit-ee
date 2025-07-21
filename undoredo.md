# Undo/Redo notes

## Current behavior
- `start_action()` records an undo snapshot only at the first modification while input is pending.
- `maybe_end_action()` finalizes the current action when the input buffer empties.
- `UNDO_DEPTH` is set to 1000.

## Remaining issues
- Further grouping logic may be needed for enter/newline and function keys.
- Persisting undo history per file is not implemented.

Relevant functions: `push_undo_state()`, `undo_action()`, `redo_action()`.
