# Undo and Redo

`ee` keeps an in-memory history of snapshots.  Each snapshot stores the
full text buffer and cursor state so a whole input chunk can be undone
at once.  The history lives only for the current session.

## How it Works

1. `run_editor()` collects key presses in short bursts.  When more than
   500 ms pass between keys or a paste is detected, the previous chunk is
   closed with `undo_end_chunk()`.
2. Editing functions call `start_action()` before modifying the buffer.
   This uses `undo_begin_chunk()` to save a snapshot of the current
   state the first time a chunk is touched.
3. `undo_action()` and `redo_action()` swap snapshots between the undo
   and redo stacks.

Large pastes are received as one chunk and therefore revert with a
single undo.  Regular typing forms its own chunks so characters undo in
order.

## Commands

- **Ctrl+K** — undo the last chunk.
- **Ctrl+R** — redo a previously undone chunk.

The stack depth is fixed at compile time (100 snapshots).  Older entries
are dropped when the limit is exceeded.

## Implementation Notes

- Input is gathered as chunks. `collect_input_chunk()` waits briefly so paste sequences arrive as a single block.
- Newlines are treated as regular characters so pasted blocks undo in one step.
- Snapshots are captured on first modification in each chunk. `undo_action()` and `redo_action()` restore snapshots and call `redraw()` to refresh the screen.
- Future work may tune buffer sizes or adopt bracketed paste mode.
