Open tasks for extended undo/redo support.

- Group pasted text as a single action. Investigate detecting paste sequences via input buffering.
- Handle newline insertion so multi-line pastes or rapid ENTER presses do not fragment history.
- Skip snapshot creation when only cursor movement occurs.
