# Unicode and UTF-8 Support Roadmap

**Problem statement**: Many non-ASCII symbols (e.g. é, ç, ñ, ł, œ, š, ž, en dash, em dash, angle quotes, CJK characters) still render incorrectly. Only umlauts appear as expected.

The goal is full UTF-8 awareness so that international text edits correctly. After merging the `utf` branch only a few characters such as German umlauts show up. Many others (ç, ñ, ž, etc.) still display as question marks. The last commits began restructuring the editor to process wide characters.

## Work completed so far

- Introduced a new `ee_char` type (`wchar_t`) and converted the text buffers and undo logic to use it.
- Switched the build to `ncursesw` and enabled locale setup with a fallback to `C.UTF-8`.
- Added multibyte conversions for search prompts and other input paths.
- Replaced ASCII-only parsing routines with `next_ascii_word` and adjusted file-name handling to stay `char` based.
- Implemented wide-character command prompts using `wget_wch` so filenames and search strings accept UTF-8 input.
- Created a helper `scan_w` to compute cursor offsets for multibyte strings.
- Verified the editor builds after these refactors.

## Roadmap

1. Finish converting line editing and drawing routines to operate entirely on `ee_char`.
2. Handle UTF‑8 decoding on file load and encode on save.
3. Adjust cursor movement and word navigation for combining characters and variable column widths.
4. Ensure screen redraws use the wide-character functions in `ncursesw`.
5. Add regression tests to verify editing typical accented Latin, Cyrillic, Greek, Arabic, Hebrew, and CJK characters.

Emoji support and other exotic glyphs remain out of scope for now.

## Outstanding gaps

- Parts of `new_curse.c` still rely on byte-oriented buffers and must be updated for wide output.
- Undo history now stores `ee_char` strings, though file I/O paths remain ASCII-only.
- Testing coverage for multi-byte sequences is minimal.

The immediate next tasks are to audit remaining byte-oriented functions, update them to use `ee_char`, and verify editing of the languages listed in the problem statement.
