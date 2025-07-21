# Undo Feature Notes

This file collects ongoing notes while refining the new chunked undo system.
The implementation reads all available input using `ioctl(FIONREAD)` so that
pasted blocks become a single undo snapshot. A redraw is forced after each undo
or redo to avoid missing text on screen.
