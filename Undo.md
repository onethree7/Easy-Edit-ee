# Undo and Redo

`ee` keeps an in-memory history of snapshots. Each snapshot stores the
entire text buffer and cursor position so a whole chunk of input can be
undone at once. The history is not saved across sessions.

## Chunk-based History

Input is processed in small bursts. `collect_input_chunk()` grabs all
queued characters and waits about **30 ms** for more so that pasted text
arrives as one block. When more than **500 ms** elapses between keys, the
current chunk is closed and a new one begins. Editing functions call
`start_action()` the first time they modify a chunk, causing
`undo_begin_chunk()` to record a snapshot.

Large pastes therefore end up as a single chunk and revert with one
undo step. Normal typing forms its own smaller chunks so characters
undo in order.

Restoring a snapshot invokes `redraw()` to repaint the whole screen.
This avoids garbled or empty lines that could appear after undoing large
pasted blocks.

## Commands

- **Ctrl+K** — undo the last chunk
- **Ctrl+R** — redo a previously undone chunk

The stack depth is fixed at compile time (100 snapshots). Older entries
are dropped when the limit is exceeded.
