# Undo Feature Notes

This file tracks progress on implementing event-based undo grouping.

- Added `is_text_input()` helper to classify characters that modify the buffer.
- Each keystroke now pushes a snapshot before processing.
- Remaining work: group pasted bursts into a single undo step.
