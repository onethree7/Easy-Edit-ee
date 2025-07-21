# Chunk-based Undo Feature Notes

This document tracks ongoing development of undo grouping. Goals:

- Group text input into undo chunks based on timing (500ms) and paste detection.
- Treat newline insertion like regular characters so pasted blocks undo together.
- Continue evaluating edge cases around function keys and command sequences.

