# Input Chunking Notes

This file tracks ongoing work on grouping paste operations for undo/redo. The
current implementation reads all available input after the first `wgetch()` call
and processes the collected characters as one chunk. Each chunk receives a
single snapshot.

Further ideas:
- Tune the buffer size and detection timing.
- Consider using bracketed paste mode if available.
