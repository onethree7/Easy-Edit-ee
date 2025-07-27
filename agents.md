# Contributor Guidelines for the `ee` editor

This document collects ideas and instructions for implementing future improvements. Follow these when adding features or refactoring the code.
After reading this guide, consult `characters.md` for the current UTF-8 plan and progress.
Refer to the module outline in [modules.md](modules.md) when moving code out of `ee.c`.

## Repository Clean‑up

- Ensure `ee` and `*.o` remain ignored by `.gitignore`.

## Build & Testing

- Use the provided `Makefile` for builds. Run `make clean && make` before committing changes to confirm the editor builds without warnings.
- For future features, consider extending `make check` to run any new automated tests or lints.

## Suggested Feature Backlog

1. **Search and Replace**
   - Implement an interactive search-and-replace command.
   - Provide prompts similar to existing search functionality.

2. **Optional Line Numbers**
   - Add a toggle to display line numbers in the text window.

3. **Syntax Highlighting**
   - Investigate using `ncurses` color pairs to highlight keywords for common languages (shell scripts, C, Markdown).
   - Make highlighting optional via the settings menu or a `.init.ee` option.

4. **Configurable Keybindings & Macros**
   - Allow users to remap common commands via a config file.

5. **Extended Undo/Redo**
   - Increase the undo stack depth and make it configurable.
   - Consider persisting undo history per file during a session.
   - *Configurable undo amount.*

6. **Color Themes**
   - Use full ncurses color support (including 256-color) and allow theme selection.

7. **Command-line Options**
   - Provide `-R` for read‑only mode and options to open at a specific line or to execute an initial command.

8. **Clipboard Integration**
   - Implement basic copy/cut/paste buffers separate from the delete/undelete logic.

9. **Tabbed Editing (Multiple Files)**
   - Add the ability to open and switch between multiple files in tabs.

10. **Testing**
    - Create a minimal test script that launches `ee` in batch mode to verify key features (opening a file, running a command, saving). This can be invoked by `make test` in the future.

## Status

Undo support with grouped input chunks is complete for now. See `undo.md` for details.
File I/O routines now reside in `fileio.c` as part of the ongoing module split.

## Coding Style

- The project uses C99. Stick to the existing lower‑case function naming and brace style.
- Address compiler warnings (mostly signedness issues) as you modify the code.
- Keep functions small and consider splitting `ee.c` into smaller source files as features grow.
- Address signedness warnings reported during compilation (e.g. in `next_word` calls). Standardize on `unsigned char` for textual data where appropriate.

---

This file serves as a to‑do list and style reference. Update it whenever a new feature is started or completed.


## Current Work

The priority is splitting the remaining logic in `ee.c` into dedicated modules.
Follow the layout in [modules.md](modules.md). UTF‑8 refactoring from
`characters.md` continues once the split is mostly done.
