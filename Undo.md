# Undo/Redo Snapshot Framework

This document describes the approach used to record user input for undo and redo
actions in `ee`.

## Snapshotting

- A **snapshot** captures the entire buffer state along with cursor and screen
  position information.
- Snapshots are taken at the start of any input event that modifies text. This
  includes character insertion, deletion, line operations and paste events.
- Navigation commands such as arrow keys do not generate snapshots.

## Input Granularity

Each physical input is grouped into an *undo chunk*:

- Consecutive key presses within 500ms of each other extend the current chunk.
- A paste operation is detected by reading the terminal buffer and always forms
  a single chunk regardless of length or newlines.

Newlines are handled as ordinary text insertion so multi‑line pastes undo in a
single step. This keeps the undo behaviour consistent when an entire block is
pasted at once.

## Stack Behaviour

- `push_undo_state()` saves a snapshot to the undo stack and clears the redo
  stack.
- `undo_action()` restores the most recent snapshot and pushes the current state
  onto the redo stack.
- `redo_action()` restores the top snapshot from the redo stack and saves the
  current state back to the undo stack.

## Implementation Notes

During input handling `collect_input_chunk()` gathers pending characters and
waits briefly (30 ms) for more to arrive. This ensures large paste operations
arrive as one array before processing begins. A timestamp check resets
`last_action` and clears the chunk flag when more than 500 ms have elapsed since
the previous input, starting a new undo group. `start_action()` takes a snapshot
the first time it is called within a chunk so the whole block can be undone with
one command.

## Development Notes

- Group text input into undo chunks using a 500 ms timeout or detected paste.
- Newlines are treated as regular text so multi-line pastes undo as one action.
- `collect_input_chunk()` reads all available input after the initial key press
  and waits 30 ms to ensure pasted data arrives as a single block.
- Future improvements may explore bracketed paste mode and finer buffer tuning.
