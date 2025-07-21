# Undo/Redo notes

Current implementation
---------------------
- `UNDO_DEPTH` is set to 1000 in `ee.c`.
- `enum action_type` tracks edits (`ACT_INSERT`, `ACT_DELETE`, etc.).
- `start_action()` pushes a snapshot when the action type changes.
- Main loop calls `push_undo_state()` before handling control or function keys so cursor moves are not undone.
- `insert_line()` now calls `start_action(ACT_INSERT)`, letting pasted text with newlines undo as one block.

Todo
----
- Detect idle periods with `wgetch` to close edit groups automatically.
- Allow the undo depth to be user configurable and persist history per file.
