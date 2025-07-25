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
- Locale initialization now verifies a UTF-8 codeset and warns if unavailable. It first tries the user's locale settings and then falls back to `C.UTF-8`, `en_US.UTF-8`, and `en_US.utf8`.
- Reworked `scanline` to use `len_char` for width calculation. This finally
  handles characters like ä ö ü Ä Ö Ü ß é ç ñ ł š ž œ æ – — „ “ « あ 日 한 你 а я α Ω ا א
  without falling back to ASCII rules. `grep -n scanline` confirms all uses
  point to the wide-aware version.
- The insert routine now uses `wcwidth` to update `scr_horz` so multi-column characters display correctly.

Example check:
```
$ grep -n scanline ee.c
252:void scanline(ee_char *pos);
860:                scanline(tp);
$ grep -n "insert(int character)" ee.c | head -n 2
250:void insert(int character);
756:insert(int character)
```

`init_locale` is invoked from the initialization routine only once:
```
$ grep -n init_locale ee.c
341:static void init_locale(void);
5173:        init_locale();
```
