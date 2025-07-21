# Chunk-based Undo Feature Notes

This document tracks ongoing development of undo grouping. Goals and notes:

- Group text input into undo chunks based on timing (500ms) and paste detection.
- Treat newline insertion like regular characters so pasted blocks undo together.
- Continue evaluating edge cases around function keys and command sequences.
- Newlines are now treated as text input in the main loop so large pastes
  revert in one step.
- `collect_input_chunk()` waits briefly to ensure pasted data is gathered as one
  chunk before any snapshot is taken.

