# Undo/Redo Framework

This document describes the current approach for grouping user input into undoable actions in **ee**.

## Event based snapshots

- Every user input event that results in text being inserted or removed is treated as a single undo unit. This includes pasted text, which arrives in one burst, and single key presses.
- Non-text commands such as cursor movement are ignored and do not trigger snapshots.
- The editor reads all pending characters from the terminal at once. When a burst of characters is detected (e.g. from a paste) the entire burst is processed as one event.

## Implementation details

1. **read_input_event()**
   - Collects all available characters from the terminal without waiting.
   - Returns them as one event buffer.
2. **is_text_input()**
   - Determines if the first character of an event will modify the buffer.
3. **undo/redo integration**
   - Before handling a text-modifying event, `push_undo_state()` is called to snapshot the buffer.
   - `start_action()` no longer triggers snapshots automatically; events control undo grouping.

This model follows similar strategies used by editors like *nano* where pasted text is grouped into a single undo step while preserving per-key undo for normal typing.
