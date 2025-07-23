# Unicode and UTF-8 Support Roadmap

This project will gradually modernize `ee` to operate correctly with UTF-8 text. All editing and display logic currently assumes single byte characters. The new work introduces wide character handling and multi-byte input.

Initial steps:

- Enable terminal UTF-8 by setting the locale and enabling ncurses `meta` mode.
- Start migrating internal buffers from `unsigned char` to `wchar_t`.
- Replace drawing routines with wide-character aware versions.
- Update file load and save paths to convert between UTF‑8 and `wchar_t`.

Full support is extensive and will require touching most of the 5k lines of `ee.c` plus the curses wrapper. Emoji and other non‑basic multilingual characters are out of scope for now.
