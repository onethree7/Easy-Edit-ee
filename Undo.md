# Modular Undo/Redo System

This document describes the redesigned undo framework used by the `ee` editor.
The goal of this rewrite is to provide reliable screen updates and a
well-structured interface so future features can easily hook into the undo
mechanism.

## Overview

Undo history is stored as a stack of *snapshots*.  Each snapshot captures the
entire text buffer along with the cursor position and scrolling offsets.
Actions that modify the buffer push a new snapshot onto the stack so the change
can be reverted.  A separate redo stack stores states that have been undone.

The implementation resides in `undo.c` and exposes the following API via
`undo.h`:

- `undo_init()` – reset all internal stacks.
- `note_input(ts, len)` – mark the time and size of the latest input chunk.
- `start_action(type)` – begin tracking a text modification.
- `push_undo_state()` – save the current editor state.
- `undo_action()` / `redo_action()` – revert or reapply a snapshot.
- `reset_chunk()` – clear the pending chunk flag.

## Input Chunking

Key presses arriving within 500 ms of each other are grouped into a single undo
chunk.  Large paste sequences are also gathered into one chunk.  This logic is
handled by `note_input()` which compares the current timestamp to the previous
input time.  When the threshold is exceeded or multiple characters arrive at
once, the chunk is closed and the next call to `start_action()` will push a new
snapshot.

`start_action()` is called at the beginning of every function that modifies the
buffer (character insertion, deletion, line editing commands and so on).  The
first call after a chunk boundary invokes `push_undo_state()` to save the old
state.  Subsequent calls within the same chunk merely update the action type so
different modifications still group together.

## Snapshot Storage

A snapshot contains a full copy of the text lines in memory.  Although storing
the entire buffer might appear wasteful, the editor manages small files and this
approach keeps the implementation straightforward.  When the undo depth reaches
`UNDO_DEPTH` (currently 1000) the oldest entry is discarded.  Undoing an action
moves the current state to the redo stack before applying the saved snapshot.
Redo operations perform the reverse.

Each snapshot includes:

```
struct snapshot {
    struct text *first;   // cloned list of lines
    struct text *curr;    // pointer to current line in the clone
    int position;         // column position
    int absolute_lin;     // cursor line number
    int scr_vert;         // vertical scroll offset
    int scr_horz;         // horizontal scroll offset
    int horiz_offset;     // left margin offset
};
```

The helper routines `clone_text_list()` and `free_text_list()` perform deep
copies and cleanup of the linked list representing the buffer.

## Screen Refresh

After applying a snapshot the editor calls `draw_screen()` followed by a helper
that touches and refreshes all windows.  This guarantees the display is
redrawn even after complex operations like multi-line undo or redo.  The helper
is internal to the module so callers simply invoke `undo_action()` or
`redo_action()` without worrying about visual updates.

## Usage Pattern

1. The main input loop collects a chunk of characters and calls `note_input()`. 
2. Editing functions invoke `start_action()` at their entry points.  If a new
   chunk started, the previous state is pushed onto the undo stack.
3. Upon user request `undo_action()` restores the last snapshot and moves the
   current state to the redo stack.  Repeating the command walks backward
   through history.  `redo_action()` reverses this process.
4. Non-modifying commands call `reset_chunk()` to ensure the next edit begins a
   fresh undo group.

This modular design keeps undo bookkeeping separate from the rest of the editor
and ensures screen contents always match the internal buffer state.
