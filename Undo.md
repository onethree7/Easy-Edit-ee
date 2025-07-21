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

Each physical input is treated as one unit:

- A normal key press stores a snapshot before inserting that character.
- A paste operation is detected by reading the terminal's input buffer and
  grouping all available characters. One snapshot is stored before the entire
  chunk is inserted.

This ensures undo/redo steps correspond directly to physical actions rather
than individual characters or lines.

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
loop. `last_action` is reset once per input chunk so that `start_action()`
stores only one snapshot for the entire group of characters.
