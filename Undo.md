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

Newlines are treated like normal text insertion so multi-line pastes undo in a
single step. This ensures undo/redo steps correspond directly to physical
actions rather than individual characters or lines.

## Stack Behaviour

- `push_undo_state()` saves a snapshot to the undo stack and clears the redo
  stack.
- `undo_action()` restores the most recent snapshot and pushes the current state
  onto the redo stack.
- `redo_action()` restores the top snapshot from the redo stack and saves the
  current state back to the undo stack.

## Implementation Notes

During input handling the editor reads all buffered characters after the first
`wgetch()` call. This groups pasted text into a single array processed in one
loop. A timestamp check resets `last_action` if more than 500ms have elapsed
since the previous input, ensuring typed characters merge into larger chunks
while pastes are kept intact. `start_action()` then stores only one snapshot for
the entire group of characters.
