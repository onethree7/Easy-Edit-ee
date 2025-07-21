# Undo Framework Notes

- Read all available characters after the first `wgetch()` call to detect paste
  sequences.
- Reset `last_action` once per collected chunk so snapshots cover the whole
  paste or single key.
- Newline characters are handled by `insert_line()` without clearing
  `last_action`, allowing an entire pasted block to undo at once.
