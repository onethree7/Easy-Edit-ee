# Undo/Redo notes

Current implementation improves grouping of edit actions.

## Implemented
- `UNDO_DEPTH` set to `1000` in `ee.c` for larger history.
- Added `enum action_type` and `last_action` to track edits.
- `start_action()` pushes a snapshot when the action type changes.
  - Called from `insert()`, `delete()`, `insert_line()` and similar.
- Control and function keys reset `last_action` to break groups without creating a snapshot.

## Limitations
- Newlines via `insert_line()` create a new group each line.
- Large pastes are still undone line by line.
- Cursor movement keys reset grouping but are reflected in the snapshot state.

## Next steps
- Detect paste/clipboard input and group as one action.
- Collapse consecutive `insert_line()` calls into a single snapshot.
- Ignore cursor-only actions in the history when no text changed.
