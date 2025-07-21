# Undo Framework Notes

- Input is gathered through `collect_input_chunk()` which waits 30 ms for more
  characters so paste sequences arrive as one block.
- Reset `last_action` once per collected chunk so snapshots cover the whole
  paste or single key.
- Newline characters are handled by `insert_line()` without clearing
  `last_action`, allowing an entire pasted block to undo at once.
