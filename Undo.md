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

`collect_input_chunk()` reads all bytes already buffered on the terminal using
`ioctl(FIONREAD)` and then sleeps briefly to catch any remaining data. This
allows huge paste operations to be ingested in a single pass. A 500 ms pause
between inputs or a paste event begins a new chunk. `start_action()` takes a
snapshot only once per chunk so the entire block can be undone with one command.

## Development Notes

- Group text input into undo chunks using a 500 ms timeout or detected paste.
- Newlines are treated as regular text so multi-line pastes undo as one action.
- `collect_input_chunk()` uses `ioctl(FIONREAD)` to read the entire terminal
  buffer and sleeps briefly to gather any leftover characters so pasted blocks
  form one snapshot.
- The editor redraws the text window after every undo and redo so the screen
  always reflects the restored state.
- Future improvements may explore bracketed paste mode and finer buffer tuning.

### Open Issues

- Long sequences of edits may consume significant memory on the undo stack.
- Further tuning of paste detection may be required for extremely slow
  terminals.
